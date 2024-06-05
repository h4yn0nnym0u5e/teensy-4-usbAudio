#include <Audio.h>

#define AUDIO_kHz ((int) AUDIO_SAMPLE_RATE / 1000)
#define AUDIO_CHANNELS USB_AUDIO_NO_CHANNELS_480

extern "C"
{
    struct usb_string_descriptor_struct
    {
        uint8_t bLength;
        uint8_t bDescriptorType;
        uint16_t wString[6+1+1+2+1];
    };
    
  usb_string_descriptor_struct usb_string_serial_number={
    2+(6+1+1+2+1)*2,3,
    {'A','u','d','i','o','-','0'+AUDIO_CHANNELS,'/','0'+(AUDIO_kHz / 10),'0' + (AUDIO_kHz % 10),'B'}
  };
}

// GUItool: begin automatically generated code
AudioSynthWaveform       wav1;           //xy=260,158
AudioSynthWaveform       wav2;           //xy=265,194
AudioSynthWaveform       wav3;           //xy=267,232
AudioSynthWaveform       wav4;           //xy=271,269
AudioSynthWaveform       wav5;           //xy=275,307
AudioSynthWaveform       wav6;           //xy=280,343
AudioSynthWaveform       wav7;           //xy=284,380
AudioSynthWaveform       wav8;           //xy=288,418
AudioInputUSBOct         usb_oct_in;     //xy=305,509
AudioRecordWAVoct        recordWAVoct;  //xy=569,599
AudioMixer4              mixer1;         //xy=575,417
AudioMixer4              mixer2;         //xy=580,489
AudioMixer4              mixer3;         //xy=717,467
AudioOutputI2S           i2sOut;         //xy=810,344
AudioOutputUSBOct        usb_oct_out;    //xy=813,249

AudioConnection          patchCord1(wav1, 0, usb_oct_out, 0);
AudioConnection          patchCord2(wav1, 0, i2sOut, 0);
AudioConnection          patchCord3(wav2, 0, usb_oct_out, 1);
AudioConnection          patchCord4(wav3, 0, usb_oct_out, 2);
AudioConnection          patchCord5(wav4, 0, usb_oct_out, 3);
AudioConnection          patchCord6(wav5, 0, usb_oct_out, 4);
AudioConnection          patchCord7(wav6, 0, usb_oct_out, 5);
AudioConnection          patchCord8(wav7, 0, usb_oct_out, 6);
AudioConnection          patchCord9(wav8, 0, usb_oct_out, 7);
AudioConnection          patchCord10(usb_oct_in, 0, mixer1, 0);
AudioConnection          patchCord11(usb_oct_in, 0, recordWAVoct, 0);
AudioConnection          patchCord12(usb_oct_in, 1, mixer1, 1);
AudioConnection          patchCord13(usb_oct_in, 1, recordWAVoct, 1);
AudioConnection          patchCord14(usb_oct_in, 2, mixer1, 2);
AudioConnection          patchCord15(usb_oct_in, 2, recordWAVoct, 2);
AudioConnection          patchCord16(usb_oct_in, 3, mixer1, 3);
AudioConnection          patchCord17(usb_oct_in, 3, recordWAVoct, 3);
AudioConnection          patchCord18(usb_oct_in, 4, mixer2, 0);
AudioConnection          patchCord19(usb_oct_in, 4, recordWAVoct, 4);
AudioConnection          patchCord20(usb_oct_in, 5, mixer2, 1);
AudioConnection          patchCord21(usb_oct_in, 5, recordWAVoct, 5);
AudioConnection          patchCord22(usb_oct_in, 6, mixer2, 2);
AudioConnection          patchCord23(usb_oct_in, 6, recordWAVoct, 6);
AudioConnection          patchCord24(usb_oct_in, 7, mixer2, 3);
AudioConnection          patchCord25(usb_oct_in, 7, recordWAVoct, 7);
AudioConnection          patchCord26(mixer1, 0, mixer3, 0);
AudioConnection          patchCord27(mixer2, 0, mixer3, 1);
AudioConnection          patchCord28(mixer3, 0, i2sOut, 1);

AudioControlSGTL5000     sgtl5000;       //xy=830,388
// GUItool: end automatically generated code

//=====================================================================
AudioSynthWaveform* wavs[] = {
  &wav1,
  &wav2,
  &wav3,
  &wav4,
  &wav5,
  &wav6,
  &wav7,
  &wav8
};

AudioMixer4* mixers[] = {&mixer1,&mixer2};

uint32_t ledOff;

void setup() 
{
  pinMode(LED_BUILTIN,OUTPUT);
  AudioMemory(150 * 128 / AUDIO_BLOCK_SAMPLES); // empirical calculation!

  while (!Serial)
    ;

  if (CrashReport)
    Serial.print(CrashReport);
    
  while (!SD.begin(BUILTIN_SDCARD))
  {
    Serial.println("SD wait...");
    delay(250);
  } 

  // At 8/96, we're generating 1,536,000 bytes/sec,
  // so a 128kB buffer will give an SD write every
  // 42.6ms - can be marginal on even a good card!
  recordWAVoct.createBuffer(128*1024,AudioBuffer::inHeap);
  
  sgtl5000.setAddress(HIGH);
  sgtl5000.enable();
  sgtl5000.volume(0.05f);

  for (int i=0;i<8;i++)
  {
    wavs[i]->begin(0.5f,220.0f + 110.0f*i,WAVEFORM_TRIANGLE);
    wavs[i]->phase(15.0f*i);
  }

  for (int i=0;i<2;i++)
  {
    for (int j=0;j<4;j++)
      mixers[i]->gain(j,0.25f);
  }

  Serial.printf("Audio block size %d samples; sample rate %.2f; %d channels\n",AUDIO_BLOCK_SAMPLES,AUDIO_SAMPLE_RATE_EXACT,AUDIO_CHANNELS);
  Serial.println("Running");
}

uint32_t lastBlocks;
int idx;
#define BUFL 20
char sbuf[BUFL], filename[BUFL+6];

void startRecording(void)
{
  sprintf(filename, "%s.wav", sbuf);
  recordWAVoct.record(filename);
  Serial.printf("Recording %s\n", filename);
}

void stopRecording(void)
{
  recordWAVoct.stop();
  Serial.println("Recording stopped");
}

void loop() 
{
  while (Serial.available() > 0)
  {
    char ch = Serial.read();
    if ('\n' == ch)
    {
      if (idx > 0)
      {
        sbuf[idx] = 0;
        startRecording();
        idx = 0;
      }
      else
        stopRecording();
        
      continue;
    }
    sbuf[idx] = ch;
    if (idx < BUFL-1)
      idx++;
  }
  
  if (millis() > ledOff)
  {
    digitalWrite(LED_BUILTIN,0);  
  }

  if (millis() - lastBlocks > 500)
  {
    const char* rec = recordWAVoct.isRecording()?"; rec":"";
    lastBlocks = millis();
    Serial.printf("Blocks %d; max %d%s\n",AudioMemoryUsage(),AudioMemoryUsageMax(),rec);
    AudioMemoryUsageMaxReset();
  }
}
