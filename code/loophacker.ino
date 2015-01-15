/*
Code by Owen Thomson (Mr Grok)
https://bitbucket.org/mrgrok
*/

// repeats what you do
const byte morseBtn = 2;
const byte modeBtn = 3;
const byte slowerBtn = 7;
const byte fasterBtn = 8;

const byte led = 9;
const byte sampleLength = 1; // 1 ms
const byte maxSamples = 100;

boolean inRecordMode = false;
boolean wasInRecordMode = false;

// buffers to record state/duration combinations
boolean states[maxSamples];
int durations[maxSamples];
int currentSampleCycles;

short idxPlayback = 0;
short idxRecord = 0;
float playbackMultiplier = 1.0; // 0.5=>double speed, 2.0=>half speed
const float speedStep = 0.0005;

void setup() {
  Serial.begin(9600);  
  pinMode(modeBtn, INPUT);
  pinMode(morseBtn, INPUT);
  pinMode(fasterBtn, INPUT);
  pinMode(slowerBtn, INPUT);
  pinMode(led, OUTPUT);
}

void resetForRecording() {
  // reset record buffers
  memset(states, 0, sizeof(states));
  memset(durations, 0, sizeof(durations));
  idxRecord = 0; // reset record idx just to make playback start point obvious 
  
  // reset playback markers
  idxPlayback=0; 
  currentSampleCycles=0;
}

void loop() {
  inRecordMode = digitalRead(modeBtn);
  if(inRecordMode == true) {
    if(!wasInRecordMode) {
      resetForRecording();      
    }
    recordLoop();
  } else {
    // continue playing loop
    playbackLoop();
  }
  
  wasInRecordMode = inRecordMode; // record prev state for next iteration so we know whether to reset the record arr index
}

void recordLoop() {
  boolean state = digitalRead(morseBtn);
  digitalWrite(led, state); // give feedback to person recording the loop
  
  if(states[idxRecord] == state) {
    // state not changed, add to duration of current state
    durations[idxRecord] += sampleLength;
  } else {
    // state changed, go to next idx and set default duration
    idxRecord++;
    if(idxRecord == maxSamples) { idxRecord = 0; } // reset idx if max array size reached
    states[idxRecord] = state;
    durations[idxRecord] = sampleLength;
  }
  
  delay(sampleLength); // slow the loop to a time constant so we can reproduce the timelyness of the recording
}

void playbackLoop()
{
  if(digitalRead(fasterBtn)) {
    playbackMultiplier-=speedStep;
  }
  if(digitalRead(slowerBtn)) {
    playbackMultiplier+=speedStep;
  }
  
  // play the loop back at the desired speed without blocking the loop
  if(currentSampleCycles == 0 && durations[idxPlayback] != 0) {
    // state changed
    digitalWrite(led, states[idxPlayback]); // set led
  }
  
  if(idxPlayback == maxSamples) { 
    // EOF recorded loop - repeat
    idxPlayback=0; 
    currentSampleCycles=0;
  } else {
    if(durations[idxPlayback]*playbackMultiplier <= currentSampleCycles) {
      // EOF current sample in recorder buffer 
      idxPlayback++; // move to next sample 
      currentSampleCycles = 0; // reset for next sample
    } else {
      // still in same sample, but in next cycle (==sampleLength approx)
      currentSampleCycles++;
    }
  }
  
  delay(sampleLength); // keep time same as we had for record loop (approximately) 
}
