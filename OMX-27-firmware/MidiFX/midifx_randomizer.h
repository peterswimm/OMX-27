#pragma once

#include "midifx_interface.h"

namespace midifx
{

    class MidiFXRandomizer : public MidiFXInterface
    {
    public:
        MidiFXRandomizer();
        ~MidiFXRandomizer() {}

        int getFXType() override;
        String getName() override;

        void loopUpdate() override;

        void onDisplayUpdate() override;

        void noteInput(MidiNoteGroup note) override;

    protected:
        void onEnabled() override;
        void onDisabled() override;

        void onEncoderChangedEditParam(Encoder::Update enc) override;
    private:
        // std::vector<MidiNoteGroup> triggeredNotes; 

        uint8_t noteMinus_ = 0;
        uint8_t notePlus_ = 0;
        uint8_t octMinus_ = 0;
        uint8_t octPlus_ = 0;
        uint8_t velMinus_ = 0;
        uint8_t velPlus_ = 0;
        uint8_t lengthPerc_ = 0;
        uint8_t chancePerc_ = 100;

        static uint8_t getRand(uint8_t v, uint8_t minus, uint8_t plus);
    };
}
