/*
    Simple audio passthrough object. Useful for bypassing other effects perhaps,
    but maybe more useful as an audio block template.
    Nolan Jome
*/

#include "Arduino.h"
#include "AudioStream.h"

class AudioEffectPassthrough : public AudioStream
{
public:
    AudioEffectPassthrough() : AudioStream(1, inputQueueArray) {};
    virtual void update(void);
private:
    audio_block_t *inputQueueArray[1];
};
