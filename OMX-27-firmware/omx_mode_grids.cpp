#include "omx_mode_grids.h"
#include "config.h"
#include "omx_util.h"
#include "omx_disp.h"
#include "omx_leds.h"
#include "sequencer.h"

using namespace grids;

enum GridModePage {
    GRIDS_DENSITY,
    GRIDS_XY,
    GRIDS_NOTES
};

OmxModeGrids::OmxModeGrids()
{
    for (int i = 0; i < 4; i++)
    {
        gridsXY[i][0] = grids_.getX(i);
        gridsXY[i][1] = grids_.getY(i);
    }
}

void OmxModeGrids::InitSetup()
{
    initSetup = true;
}

void OmxModeGrids::onModeActivated()
{
    if(!initSetup){
        InitSetup();
    }
}

void OmxModeGrids::onClockTick() {
    grids_.gridsTick();
}

void OmxModeGrids::onPotChanged(int potIndex, int potValue)
{
    if (potIndex < 4)
    {
        grids_.setDensity(potIndex, potSettings.analogValues[potIndex] * 2);
        setParam(GRIDS_DENSITY, potIndex + 1);
        omxDisp.setDirty();
    }
    else if (potIndex == 4)
    {
        int newres = (float(potSettings.analogValues[potIndex]) / 128.f) * 3;
        grids_.setResolution(newres);
        omxDisp.displayMessage((String)"Step Res " + newres);
    }
}

void OmxModeGrids::loopUpdate()
{
    auto keyState = midiSettings.keyState;

    f1_ = keyState[1] && !keyState[2];
    f2_ = !keyState[1] && keyState[2];
    f3_ = keyState[1] && keyState[2];
    fNone_ = !keyState[1] && !keyState[2];
}

void OmxModeGrids::setParam(int pageIndex, int paramPosition)
{
    int p = pageIndex * NUM_DISP_PARAMS + paramPosition;
    setParam(p);
}

void OmxModeGrids::setParam(int paramIndex)
{
    if (paramIndex >= 0)
    {
        param = paramIndex % kNumParams;
    }
    else
    {
        param = (paramIndex + kNumParams) % kNumParams;
    }
    page = param / NUM_DISP_PARAMS;
}

void OmxModeGrids::onEncoderChanged(Encoder::Update enc)
{
    if (f1_)
    {
        // Change selected param while holding F1
        if (enc.dir() < 0) // if turn CCW
        { 
            setParam(param - 1);
            omxDisp.setDirty();
        }
        else if (enc.dir() > 0) // if turn CW
        { 
            setParam(param + 1);
            omxDisp.setDirty();
        }

        return; // break;
    }

    auto amt = enc.accel(5); // where 5 is the acceleration factor if you want it, 0 if you don't)

    int paramStep = param % 5;

    if (paramStep != 0) // Page select mode if 0
    {
        switch (page)
        {
        case GRIDS_DENSITY:
        {
            int newDensity = constrain(grids_.getDensity(paramStep - 1) + amt, 0, 255);
            grids_.setDensity(paramStep - 1, newDensity);
        }
        break;
        case GRIDS_XY:
        {
            if (paramStep == 1) // Accent
            {
                int newAccent = constrain(grids_.accent + amt, 0, 255);
                grids_.accent = newAccent;
            }
            else if (paramStep == 2) // GridX
            {
                bool gridSel = false;
                for (int g = 0; g < kNumGrids; g++)
                {
                    if (gridsSelected[g])
                    {
                        int newX = constrain(grids_.getX(g) + amt, 0, 255);
                        gridsXY[g][0] = newX;
                        grids_.setX(g, newX);
                        gridSel = true;
                    }
                }
                if (!gridSel) // No grids selected, modify all
                {
                    for (int g = 0; g < kNumGrids; g++)
                    {
                        int newX = constrain(grids_.getX(g) + amt, 0, 255);
                        gridsXY[g][0] = newX;
                        grids_.setX(g, newX);
                    }
                }
            }
            else if (paramStep == 3) // GridY
            {
                bool gridSel = false;
                for (int g = 0; g < kNumGrids; g++)
                {
                    if (gridsSelected[g])
                    {
                        int newY = constrain(grids_.getY(g) + amt, 0, 255);
                        gridsXY[g][1] = newY;
                        grids_.setY(g, newY);
                        gridSel = true;
                    }
                }
                if (!gridSel) // No grids selected, modify all
                {
                    for (int g = 0; g < kNumGrids; g++)
                    {
                        int newY = constrain(grids_.getY(g) + amt, 0, 255);
                        gridsXY[g][1] = newY;
                        grids_.setY(g, newY);
                    }
                }
            }
            else if (paramStep == 4) // Chaos
            {
                int newChaos = constrain(grids_.chaos + amt, 0, 255);
                grids_.chaos = newChaos;
            }
        }
        break;
        case GRIDS_NOTES:
        {
            if (paramStep == 1)
            {
                grids_.grids_notes[0] = constrain(grids_.grids_notes[0] + amt, 0, 127);
            }
            else if (paramStep == 2)
            {
                grids_.grids_notes[1] = constrain(grids_.grids_notes[1] + amt, 0, 127);
            }
            else if (paramStep == 3)
            {
                grids_.grids_notes[2] = constrain(grids_.grids_notes[2] + amt, 0, 127);
            }
            else if (paramStep == 4)
            {
                grids_.grids_notes[3] = constrain(grids_.grids_notes[3] + amt, 0, 127);
            }
        }
        break;
        default:
            break;
        }
    }
    omxDisp.setDirty();
}

void OmxModeGrids::onEncoderButtonDown()
{
    param = (param + 1 ) % kNumParams;
    setParam(param);
}

void OmxModeGrids::onEncoderButtonDownLong()
{
    
}

bool OmxModeGrids::shouldBlockEncEdit()
{
    return false;
}

void OmxModeGrids::onKeyUpdate(OMXKeypadEvent e)
{
    int thisKey = e.key();

    // auto keyState = midiSettings.keyState;

    if (!e.held())
    {
        if (e.down() && thisKey == 0) // Aux key down
        {
            // Sequencer shouldn't be a dependancy here but current is used to advance clocks. 
            if (sequencer.playing && gridsAUX)
            {
                gridsAUX = false;
                grids_.stop();
                sequencer.playing = false;
            }
            else
            {
                gridsAUX = true;
                grids_.start();
                sequencer.playing = true;
            }
        }
        else if (e.down() && e.clicks() == 0 && (thisKey > 2 && thisKey < 11))
        {
            int patt = thisKey - 3;
            // SAVE
            // if (!keyState[1] && keyState[2])
            if (f2_)
            { 
                // F2 + PATTERN TO SAVE
                for (int k = 0; k < 4; k++)
                {
                    grids_.gridSaves[patt][k].density = grids_.getDensity(k);
                    grids_.gridSaves[patt][k].x = grids_.getX(k);
                    grids_.gridSaves[patt][k].y = grids_.getY(k);
                    // Serial.print("saved:");
                    // Serial.print(grids_wrapper.gridSaves[patt][k].density);
                    // Serial.print(":");
                    // Serial.print(grids_wrapper.gridSaves[patt][k].x);
                    // Serial.print(":");
                    // Serial.println(grids_wrapper.gridSaves[patt][k].y);
                }

                omxDisp.displayMessage((String)"Saved " + (patt + 1));
            }
            else if(fNone_)
            {
                // SELECT
                grids_.playingPattern = patt;
                for (int k = 0; k < 4; k++)
                {
                    grids_.setDensity(k, grids_.gridSaves[patt][k].density);
                    grids_.setX(k, grids_.gridSaves[patt][k].x);
                    grids_.setY(k, grids_.gridSaves[patt][k].y);
                    // Serial.print("state:");
                    // Serial.print(grids_wrapper.gridSaves[patt][k].density);
                    // Serial.print(":");
                    // Serial.print(grids_wrapper.gridSaves[patt][k].x);
                    // Serial.print(":");
                    // Serial.println(grids_wrapper.gridSaves[patt][k].y);
                }

                omxDisp.displayMessage((String)"Load " + (patt + 1));

            }
        }
    }

    if (fNone_)
    {
        // Select Grid X param
        if (e.down() && (thisKey > 10 && thisKey < 15))
        {
            gridsSelected[thisKey - 11] = true;
            setParam(GRIDS_XY, 2);
            omxDisp.setDirty();
        }
        else if (!e.down() && (thisKey > 10 && thisKey < 15))
        {
            gridsSelected[thisKey - 11] = false;
            omxDisp.setDirty();
        }

        // Select Grid Y param
        if (e.down() && (thisKey > 14 && thisKey < 19))
        {
            gridsSelected[thisKey - 15] = true;
            setParam(GRIDS_XY, 3);
            omxDisp.setDirty();
        }
        else if (!e.down() && (thisKey > 14 && thisKey < 19))
        {
            gridsSelected[thisKey - 15] = false;
            omxDisp.setDirty();
        }
    }
    if(f1_)
    {
        // Quick Select Note
        if (e.down() && (thisKey > 10 && thisKey < 15))
        {
            setParam(GRIDS_NOTES, thisKey - 10);
            omxDisp.setDirty();
        }
        // else if (!e.down() && (thisKey > 10 && thisKey < 15))
        // {
        // }

        // Select Grid Y param
        // if (e.down() && (thisKey > 14 && thisKey < 19))
        // {
        //     setParam(GRIDS_NOTES * NUM_DISP_PARAMS + (thisKey - 14));
        //     omxDisp.setDirty();
        // }
        // else if (!e.down() && (thisKey > 14 && thisKey < 19))
        // {
            
        // }
    }
}

void OmxModeGrids::onKeyHeldUpdate(OMXKeypadEvent e)
{
}

void OmxModeGrids::updateLEDs()
{
    omxLeds.updateBlinkStates();

    bool blinkState = omxLeds.getBlinkState();

    if (gridsAUX)
    {
        // Blink left/right keys for octave select indicators.
        auto color1 = blinkState ? LIME : LEDOFF;
        strip.setPixelColor(0, color1);
    }
    else
    {
        strip.setPixelColor(0, LEDOFF);
    }

    // Function Keys
    if (f3_)
    {
        auto f3Color = blinkState ? LEDOFF : FUNKTHREE;
        strip.setPixelColor(1, f3Color);
        strip.setPixelColor(2, f3Color);
    }
    else
    {
        auto f1Color = (f1_ && blinkState) ? LEDOFF : FUNKONE;
        strip.setPixelColor(1, f1Color);

        auto f2Color = (f2_ && blinkState) ? LEDOFF : FUNKTWO;
        strip.setPixelColor(2, f2Color);
    }

    updateLEDsPatterns();

    if(fNone_ || f2_) updateLEDsFNone();
    else if(f1_) updateLEDsF1();

    omxLeds.setDirty();
}

void OmxModeGrids::updateLEDsFNone()
{
    bool blinkState = omxLeds.getBlinkState();

    // uint32_t colors[8] = {};
    // colors[0] = blinkState ? MAGENTA : LEDOFF;
    // colors[1] = blinkState ? ORANGE : LEDOFF;
    // colors[2] = blinkState ? RED : LEDOFF;
    // colors[3] = blinkState ? RBLUE : LEDOFF;
    // colors[4] = blinkState ? MAGENTA : LEDOFF;
    // colors[5] = blinkState ? ORANGE : LEDOFF;
    // colors[6] = blinkState ? RED : LEDOFF;
    // colors[7] = blinkState ? RBLUE : LEDOFF;

    auto keyState = midiSettings.keyState;

    for (int k = 0; k < 4; k++)
    {
        // Change color of 4 GridX keys when pushed
        auto kColor = keyState[k + 11] ? (blinkState ? paramSelColors[k] : LEDOFF) : PINK;
        strip.setPixelColor(k + 11, kColor);
    }

    for (int k = 4; k < 8; k++)
    {
        // Change color of 4 GridY keys when pushed
        auto kColor = keyState[k + 11] ? (blinkState ? paramSelColors[k % 4] : LEDOFF) : LTCYAN;
        strip.setPixelColor(k + 11, kColor);
    }
}

void OmxModeGrids::updateLEDsF1()
{
    bool blinkState = omxLeds.getBlinkState();
    auto keyState = midiSettings.keyState;

    // updateLEDsChannelView();

    for (int k = 0; k < 4; k++)
    {
        // Change color of 4 GridX keys when pushed
        auto kColor = keyState[k + 11] ? (blinkState ? paramSelColors[k] : LEDOFF) : ORANGE;
        strip.setPixelColor(k + 11, kColor);
    }

    for (int k = 4; k < 8; k++)
    {
        strip.setPixelColor(k + 11, LEDOFF);
    }
}

void OmxModeGrids::updateLEDsChannelView()
{
    auto channelLeds = grids_.getChannelLEDS(0);

    for (int k = 0; k < 16; k++)
    {
        // Change color of 4 GridX keys when pushed
        auto level = channelLeds.levels[k] * 2; 
        auto kColor = strip.ColorHSV(0,255,level * level);
        strip.setPixelColor(k + 11, kColor);
    }
}

void OmxModeGrids::updateLEDsPatterns()
{
    int patternNum = grids_.playingPattern;

    // LEDS for top row
    for (int j = 3; j < LED_COUNT - 16; j++)
    {
        auto pColor = (j == patternNum + 3) ? seqColors[patternNum] : LEDOFF;
        strip.setPixelColor(j, pColor);
    }
}

void OmxModeGrids::setupPageLegends()
{

    // if (midiSettings.keyState[11] || midiSettings.keyState[15])
    //     {
    //         thisGrid = 0;
    //     }
    //     else if (keyState[12] || keyState[16])
    //     {
    //         thisGrid = 1;
    //     }
    //     else if (keyState[13] || keyState[17])
    //     {
    //         thisGrid = 2;
    //     }
    //     else if (keyState[14] || keyState[18])
    //     {
    //         thisGrid = 3;
    //     }

    omxDisp.clearLegends();

    omxDisp.dispPage = page + 1;

    switch (page)
    {
    case GRIDS_DENSITY:
    {
        omxDisp.legends[0] = "DS 1";
        omxDisp.legends[1] = "DS 2";
        omxDisp.legends[2] = "DS 3";
        omxDisp.legends[3] = "DS 4";
        omxDisp.legendVals[0] = grids_.getDensity(0);
        omxDisp.legendVals[1] = grids_.getDensity(1);
        omxDisp.legendVals[2] = grids_.getDensity(2);
        omxDisp.legendVals[3] = grids_.getDensity(3);
    }
    break;
    case GRIDS_XY:
    {
        int numGrids = sizeof(gridsSelected);
        int thisGrid = 0;
        int selGridsCount = 0;

        for (int i = 0; i < numGrids; i++)
        {
            if (gridsSelected[i])
            {
                selGridsCount++;
                thisGrid = i;
            }
        }

        if (selGridsCount == 0)
        {
            omxDisp.legends[1] = "X All";
            omxDisp.legends[2] = "Y All";
            thisGrid = 0;
        }
        else if (selGridsCount == 1)
        {
            // Not sure why string.c_str doesn't work
            // String l1 = "X " + (thisGrid + 1);
            // String l2 = "Y " + (thisGrid + 1);

            // char bufx[4];
            // char bufy[4];
            // snprintf(bufx, sizeof(bufx), "X %d", thisGrid + 1);
            // snprintf(bufy, sizeof(bufy), "Y %d", thisGrid + 1);

            // omxDisp.legends[1] = bufx;
            // omxDisp.legends[2] = bufy;

            // Above string code not working at all? This is ugly
            if(thisGrid == 0){
                omxDisp.legends[1] = "X 1";
                omxDisp.legends[2] = "Y 1";
            }
            else if(thisGrid == 1){
                omxDisp.legends[1] = "X 2";
                omxDisp.legends[2] = "Y 2";
            }
            else if(thisGrid == 2){
                omxDisp.legends[1] = "X 3";
                omxDisp.legends[2] = "Y 3";
            }
            else if(thisGrid == 3){
                omxDisp.legends[1] = "X 4";
                omxDisp.legends[2] = "Y 4";
            }
        }
        else
        {
            omxDisp.legends[1] = "X *";
            omxDisp.legends[2] = "Y *";
        }

        omxDisp.legends[0] = "ACNT"; // "BPM";
        omxDisp.legends[3] = "XAOS";
        omxDisp.legendVals[0] = grids_.accent; // (int)clockbpm;
        if (thisGrid != -1)
        {
            omxDisp.legendVals[1] = gridsXY[thisGrid][0];
            omxDisp.legendVals[2] = gridsXY[thisGrid][1];
        }
        omxDisp.legendVals[3] = grids_.chaos;
    }
    break;
    case GRIDS_NOTES:
    {
        omxDisp.legends[0] = "NT 1";
        omxDisp.legends[1] = "NT 2";
        omxDisp.legends[2] = "NT 3";
        omxDisp.legends[3] = "NT 4";
        omxDisp.legendVals[0] = grids_.grids_notes[0];
        omxDisp.legendVals[1] = grids_.grids_notes[1];
        omxDisp.legendVals[2] = grids_.grids_notes[2];
        omxDisp.legendVals[3] = grids_.grids_notes[3];
    }
    break;
    default:
        break;
    }
}

void OmxModeGrids::onDisplayUpdate()
{
    updateLEDs();

    if (omxDisp.isDirty())
    { // DISPLAY
        if (!encoderConfig.enc_edit)
        {
            int pselected = param % NUM_DISP_PARAMS;
            setupPageLegends();
            omxDisp.dispGenericMode(pselected);
        }
    }
}