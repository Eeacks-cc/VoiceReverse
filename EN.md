# VoiceReverse

[中文](README.md)

Use the buttons to control the "real-time" reverse play microphone recorded sound to achieve reverse voice chat. (!!ATTENTION!! The future is still in the future.)

# Build requirement

* C++ 17
* Visual Studio 2022

# OS

* Windows

# Known issues

* Sometimes waveInAddBuffer will cause an exception when trying to play in use audio data.

# Guidance

1. Download the latest Release
2. Install VB Cable driver -> [VB-Audio](https://vb-audio.com/Cable/)
3. Run VoiceReverse.exe
4. Select input device, usually the microphone
5. Select the output device, usually the input of VB Cable
6. Set reverse playback hotkey
7. Set the VB Cable output device as the audio input device in software you want to use
8. During using microphone as usual, the microphone input is transmitted to the VB Cable playback device via this software, redirected to the input device via VB Cable, and port to the software that using microphone.
9. When you hold the reverse hotkey, recording your voice, and then release the hotkey, the recorded sound is output in reverse.
10. When enabled the loopback function, the reverse sound will be looped back in real time while speaking, and the loopback will be disabled as recording.

# Mix sound

You can use virtual sound device software, for example Banana ver or Potato ver of [VoiceMeeter](https://vb-audio.com/Voicemeeter/banana.htm), which has two or more virtual inputs and outputs and mixing function, for enable loopback while recording.

Method：
1. The default audio playback device is set to VoiceMeeter virtual input1, and the output device of VoiceReverse is set to virtual input2.
2. The audio of virtual input1 in VoiceMeeter is only connect ed to the physical output device (your speaker/headphone)
3. The audio of virtual input2 in VoiceMeeter (output by VoiceReverse) is connected to both the physical output device and the virtual output device at the same time
4. The audio of the physical input line in VoiceMeeter (your microphone) is only connected to the virtual output device
5. Disable Voices Reverse loopback
6. Software that you want to use the microphone set the virtual output of VoiceMeeter as input device
7. After that, VoiceReverse will not output when in the background or recording. When playback reversed sound, only the loopback output is output via VoiceMeeter, and the sound of the actual microphone still keep output, so that it can play both forward and reverse sound.
