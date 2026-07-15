#include <Arduino.h>

inline constexpr uint32_t heapAllocatedTag = -1;

// ghetto std::any implementation that can also be stack allocated
template <uint32_t MaxSize, uint32_t Alignment = sizeof(void*)>
class whatever {
    static constexpr bool isOnHeap = MaxSize == heapAllocatedTag;

    enum Op {
        OpClone,
        OpDestroy,
        OpMoveOut,
        OpTransfer,
        OpTypeId,
        OpSizeAlignment
    };

    union InOutParams {
        void* object{};
        struct {
            usize size{};
            usize alignment{};
        } sizeAlignment;
        typeInfo typeId;
        whatever* whateverPtr;
    };

    using Storage = conditional_t<isOnHeap, void*, byte[MaxSize]>;

    void (*manager_)(Op, const whatever*, InOutParams*){};
    alignas(isOnHeap ? alignof(Storage) : Alignment) Storage data_{};

    template <typename T>
    static void ManageStorage(Op op, const whatever* instance,
                              InOutParams* inOutParams) {
        // The contained object is in the pointer
        auto objectPtr = utils::launder((T*)instance->data_);
        switch (op) {
            case OpClone:
                if constexpr (isOnHeap) {
                    inOutParams->whateverPtr->data_ =
                        new (utils::allocate(sizealignof(T))) T{*objectPtr};
                    inOutParams->whateverPtr->manager_ = instance->manager_;
                } else {
                    new (inOutParams->whateverPtr->data_) T{*objectPtr};
                    inOutParams->whateverPtr->manager_ = instance->manager_;
                }
                break;
            case OpDestroy:
                objectPtr->~T();
                break;
            case OpMoveOut:
                new (inOutParams->object) T{COMPLEX_MOVE(*objectPtr)};
                objectPtr->~T();
                break;
            case OpTransfer:
                new (inOutParams->whateverPtr->data_)
                    T{COMPLEX_MOVE(*objectPtr)};
                objectPtr->~T();
                inOutParams->whateverPtr->manager_ = instance->manager_;
                const_cast<whatever*>(instance)->manager_ = {};
                break;
            case OpTypeId:
                inOutParams->typeId = typeId(T);
                break;
            case OpSizeAlignment:
                inOutParams->sizeAlignment = {.size = sizeof(T),
                                              .alignment = alignof(T)};
                break;
        }
    }

    void allocate(decltype(manager_) newManager) {
        InOutParams newParams;
        newManager(OpSizeAlignment, nullptr, &newParams);

        if constexpr (isOnHeap) {
            InOutParams params;

            if (manager_ &&
                (manager_(OpSizeAlignment, nullptr, &params),
                 (params.sizeAlignment.alignment >=
                      newParams.sizeAlignment.alignment &&
                  params.sizeAlignment.size >= newParams.sizeAlignment.size))) {
                COMPLEX_ASSERT(data_);
                // we already have enough memory, no allocation needed
                // however we might be losing track of the current amount of
                // memory available but we'd need to store this information
                // dynamically otherwise ¯\_(ツ)_/¯
                manager_(OpDestroy, this, nullptr);
            } else {
                // we have (no) memory and it's not enough
                if (data_) {
                    COMPLEX_ASSERT(manager_);
                    utils::deallocate(data_);
                }

                data_ = utils::allocate(newParams.sizeAlignment.size,
                                        newParams.sizeAlignment.alignment);
            }
        } else {
            COMPLEX_ASSERT(Alignment >= newParams.sizeAlignment.alignment &&
                           MaxSize >= newParams.sizeAlignment.size);
        }

        manager_ = newManager;
    }

   public:
    constexpr ~whatever() {
        if (manager_) {
            COMPLEX_ASSERT(data_);
            manager_(OpDestroy, this, nullptr);
            if constexpr (isOnHeap)
                if (data_) utils::deallocate(data_);
            manager_ = {};
        }
    }

    constexpr whatever() = default;
    whatever(const whatever& other) {
        if (other.hasValue()) {
            InOutParams inOutParams;
            inOutParams.whateverPtr = this;
            other.manager_(OpClone, &other, &inOutParams);
        }
    }
    whatever(whatever&& other) noexcept {
        if (other.hasValue()) {
            InOutParams inOutParams;
            inOutParams.whateverPtr = this;
            other.manager_(OpMoveOut, &other, &inOutParams);
        }
    }
    whatever& operator=(const whatever& other) {
        *this = whatever{other};
        return *this;
    }
    whatever& operator=(whatever&& other) noexcept {
        if (this != &other) {
            this->~whatever();
            InOutParams inOutParams;
            inOutParams.whateverPtr = this;
            other.manager_(OpTransfer, &other, &inOutParams);
        }

        return *this;
    }

    template <typename T>
    whatever(T&& object) {
        using U = utils::remove_cvref_t<T>;
        static constexpr auto newManager = &ManageStorage<U>;

        allocate(newManager);
        (void)new (data_) T{COMPLEX_FWD(object)};
    }
    template <typename T>
    static whatever create(auto&&... args) {
        whatever instance;
        instance.emplace<T>(COMPLEX_FWD(args)...);
        return instance;
    }
    template <typename T>
    T& emplace(auto&&... args) {
        static constexpr auto newManager = &ManageStorage<T>;
        allocate(newManager);
        auto* ret = new (data_) T{COMPLEX_FWD(args)...};
        return *ret;
    }

    bool hasValue() const { return manager_ != nullptr; }

    typeInfo type() const {
        if (!hasValue()) return typeId(void);
        InOutParams inOutParams;
        manager_(OpTypeId, this, &inOutParams);
        return inOutParams.typeId;
    }

    void swap(whatever& other) {
        if constexpr (isOnHeap) {
            COMPLEX_SWAP_MEMBERS(manager_, other);
            COMPLEX_SWAP_MEMBERS(data_, other);
        } else {
            if (hasValue() && other.hasValue()) {
                if (this == &other) return;

                whatever temp;
                InOutParams inOutParams;

                inOutParams.whateverPtr = &temp;
                other.manager_(OpTransfer, &other, &inOutParams);
                inOutParams.whateverPtr = &other;

                manager_(OpTransfer, this, &inOutParams);
                inOutParams.whateverPtr = this;
                temp.manager_(OpTransfer, &temp, &inOutParams);
            } else if (hasValue() || other.hasValue()) {
                whatever* empty = (hasValue()) ? &other : this;
                whatever* full = (hasValue()) ? this : &other;

                InOutParams inOutParams;
                inOutParams.whateverPtr = empty;
                full->manager_(OpTransfer, full, &inOutParams);
            }
        }
    }

    template <typename... VisitorFunctions>
    usize visit(VisitorFunctions&&... visitors) {
        if (!hasValue()) return usize(-1);

        usize index = 0;
        auto probe = [this, &index]<typename T>(T&& visitor) {
            using URef = typename detail::signature<T>::type;
            using UClean = utils::remove_cvref_t<URef>;
            static_assert(utils::is_lvalue_reference_v<URef>,
                          "Parameter must an lvalue reference");

            if (manager_ == &ManageStorage<UClean>) {
                visitor(*utils::launder((T*)data_));
                return true;
            }
            ++index;
            return false;
        };

        bool isFound = (probe(COMPLEX_FWD(visitors)) || ...);
        if (!isFound) return usize(-1);
        return index;
    }

    template <typename... Ts>
    bool isOneOf() {
        usize matches = 0;
        if (hasValue()) matches = (usize(manager_ == &ManageStorage<Ts>) + ...);
        return matches > 0;
    }

    template <template <typename...> class VariantLike, typename... Ts>
    usize tryExtract(VariantLike<Ts...>& variant) {
        if (!hasValue()) return usize(-1);

        usize index = 0;
        auto probe = [&]<typename T>() {
            if (manager_ == &ManageStorage<utils::remove_cvref_t<T>>) {
                auto* object = utils::launder((T*)data_);
                variant = T{COMPLEX_MOVE(*object)};
                object->~T();

                return true;
            }
            ++index;
            return false;
        };

        bool isFound = (probe.template operator()<Ts>() || ...);
        if (!isFound) return usize(-1);
        return index;
    }

    template <typename T>
    bool tryExtract(T& variable) {
        if (manager_ == &ManageStorage<utils::remove_cvref_t<T>>) {
            auto* object = utils::launder((T*)data_);
            variable = T{COMPLEX_MOVE(*object)};
            object->~T();

            return true;
        }

        return false;
    }

    template <typename T>
    T* tryGet() {
        if (manager_ == &ManageStorage<utils::remove_cvref_t<T>>)
            return utils::launder((T*)data_);

        return nullptr;
    }

    template <typename T>
    const T* tryGet() const {
        return const_cast<whatever*>(this)->tryGet<T>();
    }
};