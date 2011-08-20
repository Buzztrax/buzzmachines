// CyanPhase AtomStereoMeld Effect

#include <windef.h>
#include <MachineInterface.h>
#include <mdk/mdk.h>


#pragma optimize ("awy", on) 

CMachineParameter const paraPolarityType =
{
            pt_byte,
            "PolarityType",
            "PolarityType",
            0,
            3,
            0xFF,
            MPF_STATE,
            0
};

CMachineParameter const paraMeld =
{
            pt_byte,
            "Meld",
            "Stereo Meld",
            0,
            0xFE,
            0xFF,
            MPF_STATE,
            127
};

CMachineParameter const paraLeftGain =
{
            pt_byte,
            "Gain-Left",
            "Left Gain",
            0,
            0xFE,
            0xFF,
            MPF_STATE,
            0xFE
};

CMachineParameter const paraMiddleGain =
{
            pt_byte,
            "Gain-Center",
            "Center Gain",
            0,
            0xFE,
            0xFF,
            MPF_STATE,
            0xFE
};

CMachineParameter const paraRightGain =
{
            pt_byte,
            "Gain-Right",
            "Right Gain",
            0,
            0xFE,
            0xFF,
            MPF_STATE,
            0xFE
};

CMachineParameter const *pParameters[] = { &paraPolarityType, &paraMeld, &paraLeftGain, &paraMiddleGain, &paraRightGain};
CMachineAttribute const *pAttributes[] = { NULL, };

#pragma pack(1)                        

class gvals
{
public:
            byte type;
            byte mix;
            byte leftgain;
            byte middlegain;
            byte rightgain;
};

#pragma pack()

CMachineInfo const MacInfo = 
{
            MT_EFFECT,
            MI_VERSION,            
            MIF_DOES_INPUT_MIXING,
            0,                                                            // min tracks
            0,                                                            // max tracks
            5,                                                            // numGlobalParameters
            0,                                                            // numTrackParameters
            pParameters,
            0,
            pAttributes,
            "CyanPhase AtomStereoMeld",                        // name
            "Stereo Meld",                                                // short name
            "Edward L. Blake",                                    // author
            "&About..."
};


class miex : public CMDKMachineInterfaceEx { };

class mi : public CMDKMachineInterface
{
public:
            mi();
            virtual ~mi();

            virtual void Tick();

            virtual void MDKInit(CMachineDataInput * const pi);
            virtual bool MDKWork(float *psamples, int numsamples, int const mode);
            virtual bool MDKWorkStereo(float *psamples, int numsamples, int const mode);
            virtual void Command(int const i);

            virtual void MDKSave(CMachineDataOutput * const po);

            virtual char const *DescribeValue(int const param, int const value);

public:
            virtual CMDKMachineInterfaceEx *GetEx() { return &ex; }
            virtual void OutputModeChanged(bool stereo) {}


public:
            miex ex;

public:
            float leftgain, middlegain, rightgain;
            float valve, antivalve;

            int type;

            gvals gval;

};


mi::mi() {  GlobalVals = &gval; }
mi::~mi() { }

void mi::MDKInit(CMachineDataInput * const pi)
{
            SetOutputMode( true ); // No mono sounds
            leftgain = 1.0f;
            middlegain = 1.0f;
            rightgain = 1.0f;

            valve = (254.0f - 127.0f)/254.0f;
            antivalve = (127.0f)/254.0f;

            type = 0;
}

void mi::MDKSave(CMachineDataOutput * const po) { }

void mi::Tick() {
            unsigned int f = 0;

            if (gval.mix != 0xFF) {
                        f = gval.mix;
                        valve = (254.0f - f)/254.0f;
                        antivalve = (f)/254.0f;            
            };

            if (gval.type != 0xFF) {
                        type = gval.type;
            };

            if (gval.leftgain != 0xFF) {
                        leftgain = (gval.leftgain / 254.0f);
            };

            if (gval.middlegain != 0xFF) {
                        middlegain = (gval.middlegain / 254.0f);
            };

            if (gval.rightgain != 0xFF) {
                        rightgain = (gval.rightgain / 254.0f);
            };

}

bool mi::MDKWork(float *psamples, int numsamples, int const mode)
{
            return false;
}

bool mi::MDKWorkStereo(float *psamples, int numsamples, int const mode)
{
            if (mode==WM_WRITE)
                        return false;
            if (mode==WM_NOIO)
                        return false;
            if (mode==WM_READ)                        // <thru>
                        return true;

            float inL, inR, leftb, midb, rightb;
            int            i;

            switch (type) {
            case 0: // +L +R
                        for( i=0; i<numsamples*2; i++ ) {

                                    inL = psamples[i];
                                    inR = psamples[i+1];


                                    leftb = (valve*inL) * leftgain;
                                    midb = ((antivalve * inL) + (antivalve * inR)) * middlegain;
                                    rightb = (valve*inR) * rightgain;

                                    psamples[i] = leftb + midb;
                                    i++;
                                    psamples[i] = rightb + midb;
                        };
                        break;
            case 1: // +L -R
                        for( i=0; i<numsamples*2; i++ ) {

                                    inL = psamples[i];
                                    inR = -(psamples[i+1]);


                                    leftb = (valve*inL) * leftgain;
                                    midb = ((antivalve * inL) + antivalve * inR) * middlegain;
                                    rightb = (valve*inR) * rightgain;

                                    psamples[i] = leftb + midb;
                                    i++;
                                    psamples[i] = rightb + midb;
                        };
                        break;
            case 2: // -L +R
                        for( i=0; i<numsamples*2; i++ ) {

                                    inL = -(psamples[i]);
                                    inR = psamples[i+1];


                                    leftb = (valve*inL) * leftgain;
                                    midb = ((antivalve * inL) + antivalve * inR) * middlegain;
                                    rightb = (valve*inR) * rightgain;

                                    psamples[i] = leftb + midb;
                                    i++;
                                    psamples[i] = rightb + midb;
                        };
                        break;
            case 3: // -L -R
                        for( i=0; i<numsamples*2; i++ ) {

                                    inL = -(psamples[i]);
                                    inR = -(psamples[i+1]);


                                    leftb = (valve*inL) * leftgain;
                                    midb = ((antivalve * inL) + antivalve * inR) * middlegain;
                                    rightb = (valve*inR) * rightgain;

                                    psamples[i] = leftb + midb;
                                    i++;
                                    psamples[i] = rightb + midb;
                        };
                        break;
            default:
                        break;
            }
            return true;
}

void mi::Command(int const i)
{
            switch (i)
            {
            case 0:
                        pCB->MessageBox("CyanPhase AtomStereoMeld 1.0\n\nCopyright 2000 Edward L. Blake\nEmail: blakee@rovoscape.com");
                        break;
            default:
                        break;
            }
}
char const *mi::DescribeValue(int const param, int const value)
{
            static char txt[16];
            switch(param)
            {
            case 0:
                        switch (value) {
                        case 0: return "+L +R"; break;
                        case 1: return "+L -R"; break;
                        case 2: return "-L +R"; break;
                        case 3: return "-L -R"; break;
                        default: return NULL; break;
                        }
                        break;
            case 1:
                        sprintf(txt,"%.1f%%", value/254.0f*100.0f );
                        return txt;
                        break;

            case 2:
                        sprintf(txt,"%.1f%%", value/254.0f*100.0f );
                        return txt;
                        break;
            case 3:
                        sprintf(txt,"%.1f%%", value/254.0f*200.0f );
                        return txt;
                        break;
            case 4:
                        sprintf(txt,"%.1f%%", value/254.0f*100.0f );
                        return txt;
                        break;

            default:
                        return NULL;
            }
}

#pragma optimize ("", on) 

