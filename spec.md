# Developing
Most of the information can be taken from the class bases in both `include/instrument` and `include/effects`.
In those folders you can also find the most barebone implementations, `simpleInstrument.hpp` and `gain.hpp`.
Those are usually the ones to look at if you want to learn the basics behind developing your own stuff for TTracker.
But becuase you want to know, here are the functions explained.

Every virtual function is supposed to be overridden becuase of custom implementations.
The code is designed to utilize the standard functions defined in the base so there will be no error if you don't override.

Something very important to remember too, you as the developer is responsible to show dopamine inducing data to the user, so that they know something is happening, even if it's bad.
By that i mean, use the functions in the `globalCalls.hpp` file. There you will find everything you need to report errors to the user, primarly through the display, if it works of course.

Something also good to remember is that the audio data is stereo, that means that the data goes like this: left sample, right sample, left sample, right sample... If you don't remember you will get a stereo sound depending on the sound you're producing but it will be at double the frequency.

### The constructor
As you see, there is a `std::map` called `data`, that is where all your data you want to save exists.
This should be used for stuff that doesn't change per sample.
It isn't illegal but not recommended because it changes anyway.
So the constructor should be used to initialize the nessecary variables for the instrument/effect.
**This is very important to remember:** Every variable has to have a unique ID being specified by the `uint16_t` in the map. 
These ID's **DO NOT** change between instrument versions.
This is because memory can be overwritten if the developer isn't keeping the same structure.
So before publishing your instrument/effect, pick a neat structure that you like. 
The constructor should be also used to allocate other arrays or buffers that your instrument/effect might be using.

### The destructor
This is quite self explanatory and is just for when the instrument/effect is replaced for a new one at runtime.
It frees **everything** and clears everything basically.
It would be nice not to have any memory leaks.

### `resize(size_t bufferSize)`
This function is called when the buffers need to be resized.
This happens only because of a tempo change because the buffer size is dependent on the samples that fit inside a single row.
The size is specified in samples for both channels combined.

### `customVariable(void* data)`
This function is for loading custom variables from the project file. There is the standard `whatever` lambdas but if the developer of the given instrument/effect would like to store their own data types, loading of those data types should be specified here. An example of this type of loading can be shown in the `basicInstrument.cpp` file.

### `render(size_t renderSize)` (Instrument only)
This function is called when rendering of the instrument should happen. 
The buffer is public so the function doesn't return anything.
The size is specified in samples for both channels combined.

### `process(int16_t* in, size_t bufferSize)` (Effect only)
This function is called when the effect has to process some kind of audio data.
The effect writes directly to the same buffer that is specified in the `in` pointer.
The size is specified in samples for both channels combined.

### `resetVoices()` (Instrument only)
This function is called to silence all voices, usually when resuming playback so that it remains silent before the actual playback

### `pressVoice(uint8_t idx, uint8_t pitch, uint8_t velocity)` (Instrument only)
This function is called when a voice is played.
The reason for the function is that every instrument can have a different way of storing voices, this is up to the developer.

### `releaseVoice(uint8_t idx)` (Instrument only)
This function is called when a voice is released.
The reason for the function is that every instrument can have a different way of storing voices, this is up to the developer.

### `updateEffect(uint8_t idx, uint8_t effectType, uint8_t effectAmount)` (Instrument only)
This function is called when an effect is triggered for the specific voice.
The reason for the function is that every instrument can have a different way of storing effects for the voices, this is up to the developer.

### `maxVoices` (Instrument only)
This variables specified the maximum amount of voices that the instrument can have. 
The maximum should be 8 and this also speicifes how many columns will be stored in the patterns.

The other functions that are left in `instrumentBase`/`effectBase` should remain the same and shouldn't be tampered with.

### The TTracker Project (.ttp) file
* * *
> 1: Project Version\
> 2: BPM\
> 2: RRB\
> 1: Track count
> > 2: Instrument type\
> > 4: Instrument data length\
> > x: Instrument data\
> > 1: Effect count
> > > 2: Effect type\
> > > 4: Effect data length\
> > > x: Effect data\
> > 256: Order table\
> > 1: Pattern count\
> > > 2: Pattern length
> > > > x: Packed pattern data

Data is stored as following: 
> 2: ID\
> x: Name\
> 4: Size\
> x: Data

As you might see, the pattern width is omitted, this is because the width is already specified in the form of the maximum amount of voices inside the instrument class.

A packed pattern data cell can have a so called "control byte" before which states which data comes next, still in order.
As for this file format it can even compess by using the fifth bit, stating that the next byte will have the amount of repeats of the current cell.