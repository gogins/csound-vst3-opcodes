# vst3-opcodes

<img src="VST_Compatible_Logo_Steinberg_with_TM_negative.png" width="100" height="100" />

## https://github.com/gogins/csound-vst3-opcodes

These are a set of Csound opcodes that enable Csound to host VST3 
plugins: both effect plugins and instrument plugins. The vst3-opcodes should 
work on Linux, Windows, and the Mac OS, and on both 64-bit and 32-bit CPU 
architectures. VST2 plugins are not supported (but the vst4cs opcodes do 
support VST2 plugins).

For users of Csound, the advantages of VST3 over VST2 are that not only 
notes, but also MIDI messages and parameter changes are scheduled with 
sample-frame accuracy. In addition, the organization of inputs, outputs, 
and parameters is more flexible.

Currently, opening a plugin's editor window from Csound, which is a feature of 
the vst4cs opcodes, is not supported.

These opcodes are licensed under the terms of the GPLv3 license,
which is compatible with both the Steinberg VST3 SDK GPLv3 license
and Csound's LGPLv2 license (which allows re-licensing Csound as 
"any later version of the GPL license").

The opcodes consist of:

  1. [vst3audio](#vst3audio)
  2. [vst3info](#vst3info)
  3. [vst3init](#vst3init)
  4. [vst3midi](#vst3midi)
  5. [vst3note](#vst3note)
  6. [vst3paramget](#vst3paramget)
  7. [vst3paramset](#vst3paramset)
  8. [vst3presetsave](#vst3presetsave)
  9. [vstpresetload](#vstpresetload)
  10. [vst3tempo](#vts3tempo)
  
The typical lifecycle of a VST3 plugin in Csound is:

  1. Load the plugin with [vst3init](#vst3init).
  2. Load a preset with [vst3presetload](#vst3presetload), 
     or (supposedly, but doesn't seem to work) select a loaded program with 
     [vst3paramset](#vst3paramset).
  3. Send notes to the plugin with [vst3note](#vst3note).
  4. Send parameter changes to the plugin with [vst3paramset](#vst3paramset).
  5. Send raw MIDI messages to the plugin with [vst3midi](#vst3midi).
  6. In a global, always-on instrument, send audio to 
     and receive audio from the plugin with [vst3audio](#vst3audio).
     
Any number of VST3 plugins may be loaded. Any number of audio channels, VST3 
parameters, or notes may be used. 

Supposedly, to change to a different program (i.e., factory-defined preset), 
examine the `vst3info` printout for a plugin and change the program using 
`vstparamset`, with the parameter ID equal to the program list ID. The 
parameter value is normalized, but will be mapped to the appropriate program 
number in the program list as follows.

_Normalize_

double normalized = program / (double) programs;

_Denormalize_

int program = min (programs, normalized * (programs + 1));

This however does not seem to work with the VST3 SDK example programs, and 
they also do not work properly in the Reaper host.

At this time the best practice for changing the preset of a VST3 plugin in 
Csound is as follows:

  1. Open the plugin in its standalone mode (if it has one) or in a VST3 host 
     such as Reaper.
  2. Edit the parameters interactively until you achieve a sound you want to 
     use. Export the current state of the parameters as a preset file.
  3. In Csound, use vst3presetload to load your custom preset file.
  4. Alternatively, simply uset vst3paramset to send all the parameter changes 
     that you need to define your preset before you play any notes, or define 
     a Csound instrument that will send such parameters for you from your 
     score.

## Examples

### vst3-opcodes-test-linux.csd
### vst3-opcodes-test-macos.csd
### vst3-opcodes-test-windows.csd

These pieces serve as both tests and examples. They assume that you have 
unzipped the csound-vst3 ZIP archive for your system into a new empty directory 
on your computer, and that you will run the example for your system from the 
`csound-vst3/vst3-opcodes` subdirectory.

```
<CsoundSynthesizer>
<CsLicense>

V S T 3   O P C O D E S   T E S T

Author: Michael Gogins

The code is licensed under the terms of the GPLv3 license.

This Csound piece demonstrates how to use the following vst3 opcodes:

  1.  vst3init
  2.  vst3info
  3.  vst3note
  4.  vst3audio
  5.  vst3paramset
  6.  vst3paramget
  7.  vst3presetsave
  8.  vst3presetload
  
These are all the opcodes actually needed to fully use VST3 instruments and 
effects in Csound. This piece also serves as basic unit tests for the VST3 
opcodes.

The piece consists of an algorithmically generated score, which is rendered 
with several of the instruments in the VST3 SDK; effects are added using 
several other plugins in the VST3 SDK. Parameters and presets also are used.

Assuming you are on Linux, have installed Csound, ahd have unzipped the 
csound-vst3-linux.zip file into a new empty directory, then things are set up
to run this piece from the `csound-vst3/vst3-opcodes` subdirectory.

</CsLicense>
<CsOptions>
-m195 --opcode-lib="../../build-linux/lib/Debug/libvst3_plugins.so" -z1
</CsOptions>
<CsInstruments>

sr      = 48000
ksmps   = 100
nchnls  = 2
0dbfs   = 20

connect "JX10_Output", "outleft", "Delay", "inleft"
connect "JX10_Output", "outright", "Delay", "inright"
connect "Piano_Output", "outleft", "Delay", "inleft"
connect "Piano_Output", "outright", "Delay", "inright"
connect "Delay", "outleft", "Reverb", "inleft"
connect "Delay", "outright", "Reverb", "inright"
connect "Reverb", "outleft", "Master_Output", "inleft"
connect "Reverb", "outright", "Master_Output", "inright"

alwayson "JX10_Output"
alwayson "Piano_Output"
alwayson "Delay"
alwayson "Reverb"
alwayson "Master_Output"

gi_vst3_handle_jx10 vst3init "../../build-linux/VST3/Debug/mda-vst3.vst3", "mda JX10", 1
vst3info gi_vst3_handle_jx10

gi_vst3_handle_piano vst3init "../../build-linux/VST3/Debug/mda-vst3.vst3", "mda Piano", 1
vst3info gi_vst3_handle_piano

gi_vst3_handle_delay vst3init "../../build-linux/VST3/Debug/mda-vst3.vst3", "mda Delay", 1
vst3info gi_vst3_handle_delay

gi_vst3_handle_ambience vst3init "../../build-linux/VST3/Debug/mda-vst3.vst3", "mda Ambience", 1
vst3info gi_vst3_handle_ambience

// Array of instrument plugins indexed by instrument number, for sending 
// parameter changes.

gi_plugins[] init 5
gi_plugins[3] init gi_vst3_handle_piano
gi_plugins[4] init gi_vst3_handle_jx10

// Score generating instrument.

gi_iterations init 50
gi_duration init 1.8
gi_time_step init .6666667
gi_loudness init 70
instr Score_Generator
i_time = p2
i_instrument = p4
i_c = p5
i_y = p6
i_bass = p7
i_range = p8
i_time_step = 1 / 8
i_iteration = 0
while i_iteration < gi_iterations do
    i_iteration = i_iteration + 1
    i_time = p2 + (i_iteration * gi_time_step)
    // Normalized logistic equation:
    i_y1 = i_c * i_y * (1 - i_y) * 4
    i_y = i_y1
    i_pitch = floor(i_bass + (i_y * i_range)) 
    event_i "i", i_instrument, i_time, gi_duration, i_pitch, gi_loudness
    prints "   %f => i %f %f %f %f %f\n", i_y, i_instrument, i_time, gi_duration, i_pitch, gi_loudness
od
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Param_Change
i_target_plugin = p4
i_vst3_plugin init gi_plugins[p4]
k_parameter_id init p5
k_parameter_value init p6
vst3paramset i_vst3_plugin, k_parameter_id, k_parameter_value
prints "%-24.24s i %9.4f t %9.4f d %9.4f target: %3d  id: %3d  value: %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p6, active(p1)
endin

instr Piano
i_note_id vst3note gi_vst3_handle_piano, 0, p4, p5, p3
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr JX10
i_note_id vst3note gi_vst3_handle_jx10, 0, p4, p5, p3
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Piano_Output
a_out_left, a_out_right vst3audio gi_vst3_handle_piano 
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr JX10_Output
a_out_left, a_out_right vst3audio gi_vst3_handle_jx10 
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Print_Info
i_target_plugin = p4
i_vst3_plugin init gi_plugins[p4]
vst3info i_vst3_plugin
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Save_Preset
i_target_plugin = p4
S_preset_name init p5
i_vst3_plugin init gi_plugins[p4]
vst3presetsave i_vst3_plugin, S_preset_name
prints "%-24.24s i %9.4f t %9.4f d %9.4f target: %3d  preset: %s #%3d\n", nstrstr(p1), p1, p2, p3, i_target_plugin, S_preset_name, active(p1)
endin

instr Program_Change
i_target_plugin = p4
i_vst3_plugin init gi_plugins[p4]
; May only be relevant to the MDA example plugins.
k_parameter_id init 1886548852 
k_parameter_value init p5
vst3paramset i_vst3_plugin, k_parameter_id, k_parameter_value
prints "Don't expect this one to work yet!\n"
prints "%-24.24s i %9.4f t %9.4f d %9.4f target: %3d  id: %3d  value: %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, i_target_plugin, k_parameter_id, k_parameter_value, active(p1)
endin

instr Load_Preset
i_target_plugin = p4
S_preset_name init p5
i_vst3_plugin init gi_plugins[p4]
vst3presetload i_vst3_plugin, S_preset_name
prints "%-24.24s i %9.4f t %9.4f d %9.4f target: %3d  preset: %s #%3d\n", nstrstr(p1), p1, p2, p3, i_target_plugin, S_preset_name, active(p1)
endin

instr Delay
a_in_left inleta "inleft"
a_in_right inleta "inright"
a_out_left, a_out_right vst3audio gi_vst3_handle_delay, a_in_left, a_in_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Reverb
a_in_left inleta "inleft"
a_in_right inleta "inright"
k_old_size vst3paramget gi_vst3_handle_ambience, 0
vst3paramset gi_vst3_handle_ambience, 0, .95
vst3paramset gi_vst3_handle_ambience, 3, .9
vst3presetsave gi_vst3_handle_ambience, "ambience.preset"
a_out_left, a_out_right vst3audio gi_vst3_handle_ambience, a_in_left, a_in_right
outleta "outleft", a_out_left
outleta "outright", a_out_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

instr Master_Output
a_in_left inleta "inleft"
a_in_right inleta "inright"
outs a_in_left, a_in_right
prints "%-24.24s i %9.4f t %9.4f d %9.4f k %9.4f v %9.4f p %9.4f #%3d\n", nstrstr(p1), p1, p2, p3, p4, p5, p7, active(p1)
endin

</CsInstruments>
<CsScore>
f 0 40
i "Score_Generator" 1 1 3 .989 .5 36 60
i "Score_Generator" 1 1 4 .989 .5 78 6
; Stores original parameter state...
i "Print_Info" 1.1 1 4
i "Save_Preset" 1.2 1 4 "jx10.vstpreset"
i "Print_Info" 1.3 1 4
; Changes filter state...
i "Param_Change" 10 1 4 1 .15
i "Param_Change" 10 1 4 7 .15
i "Param_Change" 10 1 4 12 .1
i "Print_Info" 10.5 1 4
; Restores original parameter state.
i "Load_Preset" 25 1 4 "jx10.vstpreset"
i "Print_Info" 25.5 1 4
;i "Program_Change" 25 1 4 12
;i "Print_Info" 30.0 1 3
;i "Program_Change" 30.1 1 3 4
;i "Print_Info" 30.2 1 3

</CsScore>
</CsoundSynthesizer>```

## vst3audio

vst3-opcodes -- VST plugin hosting in Csound.

### Description

**vst3audio** sends audio to, and/or receives audio from, a VST3 plugin.

### Syntax

[a_audio_output_1,...] **vst3audio** i_handle [, a_audio_input_1,...]

### Initialization

*i_handle* -- the handle that identifies the plugin, obtained from 
[vst3init](#vst3init).

### Performance

*a_audio_input_n* -- one of zero or more, up to 32, audio input channels.

*a_audio_output_n* -- one of zero or more, up to 32, auto output channels.

This opcode is used for both VST instruments and VST effects. For instruments, 
inputs are allowed but not required, and may or may not actually be handled by 
the plugin.

Note that the mininum of the plugin inputs and the opcode inputs, and the 
minimum of the plugin outputs and the opcode outputs, are used. It is assumed 
that the "Main" audio and event busses in the plugin are the first input 
busses and first output busses.

The audio sent to the plugin must come from some other source in Csound, such
as an input opcode or a Csound buss channel, and the audio received from the 
plugin must be sent to some other sink in Csound, such as an opcode output or 
a Csound buss channel.

### Examples

See [vst3-opcodes.csd](#example).

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midi](#vst3midi), 
[vst3note](#vst3note),
[vst3paramget](#vst3paramget), 
[vst3paramset](#vst3paramset),
[vst3presetload](#vst3presetload), 
[vst3presetsave](#vst3presetsave), 
[vts3tempo](#vts3tempo)

### Credits

Michael Gogins<br>
http://michaelgogins.tumblr.com<br>
michael dot gogins at gmail dot com<br>

## vst3info

vst3-opcodes -- VST plugin hosting in Csound.

### Description

**vst3info** prints information about the plugin module, input and output 
busses, parameters, presets, and program lists. Not all plugins provide 
all types of information.

### Syntax

**vst3info** i_handle

### Initialization

*i_handle* -- the handle that identifies the plugin, obtained from 
[vst3init](#vst3init).

### Examples

See [vst3-opcodes.csd](#example).

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init), 
[vst3midi](#vst3midi), 
[vst3note](#vst3note), 
[vst3paramget](#vst3paramget), 
[vst3paramset](#vst3paramset), 
[vst3presetload](#vst3presetload), 
[vst3presetsave](#vst3presetsave), 
[vts3tempo](#vts3tempo)

### Credits

Author:

Michael Gogins<br>
http://michaelgogins.tumblr.com<br>
michael dot gogins at gmail dot com<br>

## vst3init

vst3-opcodes -- VST3 plugin hosting in Csound.

### Description

**vst3init** loads a VST3 plugin into memory for use with the 
other vst3-opcodes. Both VST3 effects and instruments (synthesizers) can be 
used. 

Note that for VST3, there may be multiple plugins defined in one loadable 
module.

### Syntax

i_handle **vst3init** S_module_pathname, S_plugin_name [,i_verbose]

### Initialization

*i_handle* -- the handle that identifies the plugin, to be passed to the other 
vst3opcodes that use the plugin.

*S_module_pathname* -- the pathname of a ".vst3" directory that contains a 
plugin module and its resources. Remember to use '/' instead of '\\' as the 
path separator.

*S_plugin_name* -- the name of a plugin within the module. If you do not know 
the name of the plugin, run with *i_verbose" set to true to find the name.

*i_verbose* -- print a list of all the plugins defined in the module, as well 
as other information.

### Examples

See [vst3-opcodes.csd](#example).

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midi](#vst3midi), 
[vst3note](#vst3note),
[vst3paramget](#vst3paramget), 
[vst3paramset](#vst3paramset),
[vst3presetload](#vst3presetload), 
[vst3presetsave](#vst3presetsave), 
[vts3tempo](#vts3tempo)

### Credits

Author:

Michael Gogins<br>
http://michaelgogins.tumblr.com<br>
michael dot gogins at gmail dot com<br>

## vst3midi

vst3-opcodes -- VST plugin hosting in Csound.

### Description

**vst3midi** sends raw MIDI messages to a VST3 plugin. Essentially, 
there are all MIDI messages that can be sent as 2 or 3 bytes. System 
exclusive messages are not handled.

### Syntax

**vst3midi** i_handle, k_status, k_channel, k_data_1, k_data_2

### Initialization

*i_handle* -- the handle that identifies the plugin, obtained from 
[vst3init](#vst3init).

### Performance

*k_status* -- The status code of the MIDI channel message.

*k_channel* -- The zero-based MIDI channel of the message.

*k_data_1* -- The first data byte of the message.

*k_data_2* -- The second data byte of the message.

Please note, the ranges and semantics of these numbers are defined by the 
MIDI 1.0 Standard. The messages are scheduled immediately. If any of these 
parameters changes during performance, a new MIDI channel message is 
immediately sent. The channel parameter is simply added to the status 
parameter; thus, if the status code already specifies the channel number, 
then the *k_channel* parameter should be set to zero.

### Examples

See [vst3-opcodes.csd](#example).

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midi](#vst3midi), 
[vst3note](#vst3note),
[vst3paramget](#vst3paramget), 
[vst3paramset](#vst3paramset),
[vst3presetload](#vst3presetload), 
[vst3presetsave](#vst3presetsave), 
[vts3tempo](#vts3tempo)

### Credits

Author:

Michael Gogins<br>
http://michaelgogins.tumblr.com<br>
michael dot gogins at gmail dot com<br>

## vst3note

vst3-opcodes -- VST plugin hosting in Csound.

### Description

**vst3note** sends a single note with a specified duration to a VST3 
plugin. The note is translated to a MIDI Note On channel message with a 
matching MIDI Note Off channel message. 

The VST3 protocol supports fractional pitches, which are sent as cents of 
detuning from the integer MIDI key number. In this way, any pitch can be sent 
to a plugin, which may or may not be equipped to render it.

### Syntax

i_note_id **vst3note** i_handle, i_midi_channel, i_midi_key, i_midi_velocity, i_duration

### Initialization

*i_note_id* -- An identifier, unique for this instance of this plugin, of this 
note, possibly useful for per-note control.

*i_handle* -- the handle that identifies the plugin, obtained from 
[vst3init](#vst3init).

*i_midi_channel* -- The zero-based MIDI channel in the interval [0, 15] of the 
message.

*i_midi_key* -- The real-valued MIDI key number in the interval [0, 127] of 
the note. It may have a fractional value that will be translated to a VST3 
detuning parameter. Middle "C" is key number 60.

*i_midi_velocity* -- The MIDI velocity of the note in the interval [0, 127]. 
Mezzo-forte is velocity number 80.

*i_duration* -- The real-valued duration of the note in beats (by default, 1 beat in 
Csound is 1 second).

Note: Be sure the instrument containing **vst3note** is not finished before the 
duration of the note, otherwise you'll have a 'hung' note.

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midi](#vst3midi), 
[vst3note](#vst3note),
[vst3paramget](#vst3paramget), 
[vst3paramset](#vst3paramset),
[vst3presetload](#vst3presetload), 
[vst3presetsave](#vst3presetsave), 
[vts3tempo](#vts3tempo)

### Credits

Author:

Michael Gogins<br>
http://michaelgogins.tumblr.com<br>
michael dot gogins at gmail dot com<br>

## vst3paramget

vst3-opcodes -- VST plugin hosting in Csound.

### Description

**vst3paramget** gets the current value of a single parameter from a VST3 
plugin.

### Syntax

k_value **vst3paramset** i_handle, k_parameter

### Initialization

*i_handle* -- the handle that identifies the plugin, obtained from 
[vst3init](#vst3init).

### Performance

*k_parameter* -- the identification number of the parameter. This can be 
obtained from the plugin's documentation, or by using [vst3info](#vst3info). 
Note that this must be the id number of the parameter, not its index in the 
list of parameters.

*k_value* -- the value of the parameter, a real number in the interval [0, 1].
Most parameters have default values that can be printed by using 
[vst3info](#vst3info). 

### Examples

See [vst3-opcodes.csd](#example).

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midi](#vst3midi), 
[vst3note](#vst3note),
[vst3paramget](#vst3paramget), 
[vst3paramset](#vst3paramset),
[vst3presetload](#vst3presetload), 
[vst3presetsave](#vst3presetsave), 
[vts3tempo](#vts3tempo)

### Credits

Author:

Michael Gogins<br>
http://michaelgogins.tumblr.com<br>
michael dot gogins at gmail dot com<br>

## vst3paramset

vst3-opcodes -- VST plugin hosting in Csound.

### Description

**vst3paramset** sets the current value of a single parameter in a VST3 
plugin. This is also used for selecting a preset from a factory list of 
presets, or a list of presets that is loaded by the plugin itself.

### Syntax

**vst3paramset** i_handle, k_parameter, k_value

### Initialization

*i_handle* -- the handle that identifies the plugin, obtained from 
[vst3init](#vst3init).

### Performance

*k_parameter* -- the identification number of the parameter. This can be 
obtained from the plugin's documentation, or by using [vst3info](#vst3info).
Note that this must be the id number of the parameter, not its index in the 
list of parameters.

*k_value* -- the value of the parameter, a real number in the interval [0, 1].
Most parameters have default values that can be printed by using 
[vst3info](#vst3info). 

If either of these arguments changes during performance, the plugin is 
immediately updated with a new parameter value.

### Examples

See [vst3-opcodes.csd](#example).

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midi](#vst3midi), 
[vst3note](#vst3note),
[vst3paramget](#vst3paramget), 
[vst3paramset](#vst3paramset),
[vst3presetload](#vst3presetload), 
[vst3presetsave](#vst3presetsave), 
[vts3tempo](#vts3tempo)

### Credits

Author:

Michael Gogins<br>
http://michaelgogins.tumblr.com<br>
michael dot gogins at gmail dot com<br>

## vst3presetload

vst3-opcodes -- VST plugin hosting in Csound.

### Description

**vst3presetload** loads a preset from a file. A preset consists 
of a component (instrument or effect) identifier, the value of each parameter in the preset, and 
possibly other data such as a program list.

### Syntax

**vst3presetload** i_handle, S_preset_filepath

### Initialization

*i_handle* -- the handle that identifies the plugin, obtained from 
[vst3init](#vst3init).

*S_preset_filepath* -- the full pathname of the preset file. Remember to 
use '/' instead of '\\' as the path separator.

### Examples

See [vst3-opcodes.csd](#example).

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midi](#vst3midi), 
[vst3note](#vst3note),
[vst3paramget](#vst3paramget), 
[vst3paramset](#vst3paramset),
[vst3presetload](#vst3presetload), 
[vst3presetsave](#vst3presetsave), 
[vts3tempo](#vts3tempo)

### Credits

Author:

Michael Gogins<br>
http://michaelgogins.tumblr.com<br>
michael dot gogins at gmail dot com<br>

## vst3presetsave

vst3-opcodes -- VST plugin hosting in Csound.

### Description

**vst3presetsave** saves a plugin preset to a file. The preset file stores the 
identifier of the instrument or effect, the identifiers, names, and values of 
all the parameters in the program, and possibly other information such as a 
program list.

### Syntax

**vst3presetsave** i_handle, S_preset_filepath

### Initialization

*i_handle* -- the handle that identifies the plugin, obtained from 
[vst3init](#vst3init).

*S_preset_filepath* -- the full pathname of the preset file. Remember to 
use '/' instead of '\\' as the path separator.

### Examples

See [vst3-opcodes.csd](#example).

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midi](#vst3midi), 
[vst3note](#vst3note),
[vst3paramget](#vst3paramget), 
[vst3paramset](#vst3paramset),
[vst3presetload](#vst3presetload), 
[vst3presetsave](#vst3presetsave), 
[vts3tempo](#vts3tempo)

### Credits

Author:

Michael Gogins<br>
http://michaelgogins.tumblr.com<br>
michael dot gogins at gmail dot com<br>

### vst3tempo

vst3-opcodes -- VST plugin hosting in Csound.

### Description

**vst3tempo** changes the tempo in a VST3 plugin.

### Syntax

**vst3note** i_handle, k_beats_per_minute

### Initialization

*i_handle* -- the handle that identifies the plugin, obtained from 
[vst3init](#vst3init).

### Performance

*k_beats_per_minute* -- the musical tempo in beats per minute. 

By default, Csound's musical tempo is 1 beat per second, so that note onsets 
and durations can be specified in seconds. But Csound's musical tempo can be 
changed and, if it does change, the plugin's tempo should be changed to 
match. Tempo changes can occur at k-rate.

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midi](#vst3midi), 
[vst3note](#vst3note),
[vst3paramget](#vst3paramget), 
[vst3paramset](#vst3paramset),
[vst3presetload](#vst3presetload), 
[vst3presetsave](#vst3presetsave), 
[vts3tempo](#vts3tempo)

### Credits

Author:

Michael Gogins<br>
http://michaelgogins.tumblr.com<br>
michael dot gogins at gmail dot com<br>
