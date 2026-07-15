#ifndef LINKEDLIST_H
#define LINKEDLIST_H

#include <Arduino.h>

#include "esp_heap_caps.h"

template <typename itemTemplate>
struct linkedItem {
    linkedItem* prev;
    itemTemplate value;
    linkedItem* next;
};

template <typename listTemplate>
class linkedList {
   public:
    linkedItem<listTemplate>* iterate(const size_t idx) {
        if (idx >= size) return nullptr;
        if (idx == 0) return first;
        if (idx == size - 1) return last;
        linkedItem<listTemplate>* iter = first;
        if (idx > size >> 1) {
            iter = last;
            for (size_t i = size - 1; i > idx; i--) iter = iter->prev;
        } else {
            for (size_t i = 0; i < idx; i++) iter = iter->next;
        }
        return iter;
    }

    void push_back(listTemplate value) {
        auto* ptr = static_cast<linkedItem<listTemplate>*>(heap_caps_malloc(
            sizeof(linkedItem<listTemplate>), MALLOC_CAP_SPIRAM));
        ptr->value = value;
        if (size == 0) {
            first = last = ptr;
            size++;
            return;
        }
        ptr->prev = last;
        last->next = ptr;
        last = ptr;
        size++;
    }

    void pop_back() {
        if (size == 0) return;
        first->next->prev = nullptr;
        size--;
        free(first);
    }

    void push_front(listTemplate value) {
        auto* ptr = static_cast<linkedItem<listTemplate>*>(heap_caps_malloc(
            sizeof(linkedItem<listTemplate>), MALLOC_CAP_SPIRAM));
        ptr->value = value;
        if (size == 0) {
            first = last = ptr;
            size++;
            return;
        }
        ptr->next = first;
        first->prev = ptr;
        first = ptr;
        size++;
    }

    void pop_front() {
        if (size == 0) return;
        last->prev->next = nullptr;
        size--;
        free(last);
    }

    void insert(listTemplate value, const size_t idx) {
        if (idx == 0) {
            push_front(value);
            return;
        }
        if (idx == size) {
            push_back(value);
            return;
        }
        linkedItem<listTemplate>* iter = iterate(idx);
        if (iter == nullptr) return;
        auto* ptr = static_cast<linkedItem<listTemplate>*>(heap_caps_malloc(
            sizeof(linkedItem<listTemplate>), MALLOC_CAP_SPIRAM));
        ptr->value = value;
        ptr->next = iter;
        ptr->prev = iter->prev;
        iter->prev->next = ptr;
        iter->prev = ptr;
        size++;
    }

    void remove(const size_t idx) {
        if (idx == 0) {
            pop_front();
            return;
        }
        if (idx == size - 1) {
            pop_back();
            return;
        }
        linkedItem<listTemplate>* iter = iterate(idx);
        if (iter == nullptr) return;
        iter->prev->next = iter->next;
        iter->next->prev = iter->prev;
        size--;
        free(iter);
    }

    void clear() {
        if (size == 0) return;
        linkedItem<listTemplate>* iter = first;
        while (iter->next != nullptr) {
            iter = iter->next;
            free(iter->prev);
        }
        free(iter);
        size = 0;
        first = last = nullptr;
    }

   private:
    linkedItem<listTemplate>* first = nullptr;
    linkedItem<listTemplate>* last = nullptr;
    size_t size = 0;
};

#endif