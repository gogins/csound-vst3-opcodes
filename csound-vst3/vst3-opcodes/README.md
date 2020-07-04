# vst3-opcodes

<img src="VST_Compatible_Logo_Steinberg_with_TM_negative.png" width="100" height="100" />

## https://github.com/gogins/csound-vst3-opcodes

These are a set of Csound opcodes that enable Csound to host VST3 
plugins, both effect plugins and instrument plugins. The vst3-opcodes should 
work on Linux, Windows, and OS X, and on both 64-bit and 32-bit CPU 
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
  4. [vst3midiout](#vst3midiout)
  5. [vst3note](#vst3note)
  6. [vst3paramget](#vst3paramget)
  7. [vst3paramset](#vst3paramset)
  8. [vst3presetsave](#vst3presetsave)
  9. [vst3presetset](#vst3presetset)
  10. [vts3tempo](#vts3tempo)
  
The typical lifecycle of a VST3 plugin in Csound is:

  1. Load the plugin with [vst3init](#vst3init).
  2. Load a preset with [vst3presetload](#vst3presetload), 
     or select a loaded program with [vst3paramset](#vst3paramset).
  3. Send notes to the plugin with [vst3note](#vst3note).
  4. Send parameter changes to the plugin with [vst3paramset](#vst3paramset).
  5. Send MIDI channel messages to the plugin with [vst3midiout](#vst3midiout).
  6. In a global, always-on instrument, send audio to 
     and receive audio from the plugin with [vst3audio](#vst3audio).
     
Any number of VST3 plugins may be loaded. Any number of audio channels, VST3 
parameters, or notes may be used. 

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

*a_audio_output_n* -- one of zero or more audio output channels.

Note that the mininum of the plugin inputs and the opcode inputs, and the 
minimum of the plugin outputs and the opcode outputs, are used. It is assumed 
that the main audio busses in the plugin are the first input buss and first 
output buss.

The audio sent to the plugin must come from some other source in Csound, such
as an input opcode or a Csound buss channel, and the audio received from the 
plugin must be sent to some other sink in Csound, such as an opcode output or 
a Csound buss channel.

### Examples

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midiout](#vst3midiout), 
[vst3note](#vst3note),
[vst3paramget](#vst3paramget), 
[vst3paramset](#vst3paramset),
[vst3presetload](#vst3presetload), 
[vst3presetsave](#vst3presetsave), 
[vts3tempo](#vts3tempo)

### Credits

Michael Gogins 
http://michaelgogins.tumblr.com
michael dot gogins at gmail dot com

## vst3info

vst3-opcodes -- VST plugin hosting in Csound.

### Description

**vst3info** prints information about the plugin module, input and output 
busses, parameters, presets, and presets.

### Syntax

**vst3info** i_handle

### Initialization

*i_handle* -- the handle that identifies the plugin, obtained from 
[vst3init](#vst3init).

### Examples

### See Also

[vst3audio](#vst3audio), 

[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midiout](#vst3midiout), 
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
used. Note that for VST3, there may be multiple plugins defined in one 
loadable module.

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

*i_verbose* -- print a list of all the plugins defined in the module.

### Examples

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midiout](#vst3midiout), 
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

## vst3midiout

vst3-opcodes -- VST plugin hosting in Csound.

### Description

**vst3midiout** sends MIDI channel messages to a VST3 plugin.

### Syntax

**vst3midiout** i_handle, k_status, k_channel, k_data_1, k_data_2

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
immediately sent.

### Examples

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midiout](#vst3midiout), 
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

*i_midi_channel* -- The zero-based MIDI channel of the message.

*i_midi_key* -- The real-valued MIDI key number of the note. It may have a 
fractional value that will be translated to a VST3 detuning parameter. Middle 
"C" is key number 60.

*i_midi_velocity* -- The MIDI velocity of the note. Mezzo-forte is 80.

*i_duration* -- The real-valued duration of the note in beats (by default, 1 beat in 
Csound is 1 second).

Note: Be sure the instrument containing **vst3note** is not finished before the 
duration of the note, otherwise you'll have a 'hung' note.

### Examples

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midiout](#vst3midiout), 
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

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midiout](#vst3midiout), 
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

### See Also

[vst3audio](#vst3audio), 

[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midiout](#vst3midiout), 
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
of a program identifier plus the value of each parameter in the preset.

### Syntax

**vst3presetload** i_handle, S_preset_filepath

### Initialization

*i_handle* -- the handle that identifies the plugin, obtained from 
[vst3init](#vst3init).

*S_preset_filepath* -- the full pathname of the preset file. Remember to 
use '/' instead of '\\' as the path separator.

### Examples

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midiout](#vst3midiout), 
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

**vst3presetsave** saves a plugin preset to a file. The preset consists of the 
identifier of the current program plus the names and values of all the 
parameters in the program.

### Syntax

**vst3presetsave** i_handle, S_preset_filepath

### Initialization

*i_handle* -- the handle that identifies the plugin, obtained from 
[vst3init](#vst3init).

*S_preset_filepath* -- the full pathname of the preset file. Remember to 
use '/' instead of '\\' as the path separator.

### Examples

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midiout](#vst3midiout), 
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

### Examples

### See Also

[vst3audio](#vst3audio), 
[vst3info](#vst3info), 
[vst3init](#vst3init),
[vst3midiout](#vst3midiout), 
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
