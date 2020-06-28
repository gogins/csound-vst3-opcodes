# vst3-opcodes

<img src="VST_Compatible_Logo_Steinberg_with_TM_negative.png" width="100" height="100" />

## https://github.com/gogins/csound-vst3-opcodes

These are a set of Csound opcodes that enable Csound to host VST3 
plugins, both effect plugins and instrument plugins. The VST2 
protocol, implemented by the vst4cs opcodes, is not supported. The 
opcodes should work on Linux, Windows, and OS X, and on both 
64-bit and 32-bit CPU architectures.

These opcodes are licensed under the terms of the GPLv3 license,
which is compatible with both the Steinberg VST3 SDK GPLv3 license
and Csound's LGPLv2 license (which allows re-licensing Csound as 
"any later version of the GPL license").

Typically, each VST plugin is loaded using the vst3init opcode,
notes are sent using vst3note, MIDI channel messages of any kind 
are sent using vst3midiout, VST parameter changes are sent and 
received using vst3paramset and vst3paramget, and so on. Audio
must be sent and received from a global, always-on instance of 
the vst3audio opcode. Any number of audio channels, VST parameters, 
or notes may be used. Any number of VST plugins may be loaded. 

## vst3bankload

vst3-opcodes -- VST plugin hosting in Csound.

### Description

### Syntax

### Initialization

### Performance

### Examples

### See Also

### Credits

Michael Gogins 
http://michaelgogins.tumblr.com
michael dot gogins at gmail dot com

## vst3banksave

vst3-opcodes -- VST plugin hosting in Csound.

### Description

### Syntax

### Initialization

### Performance

### Examples

### See Also

### Credits

Michael Gogins 
http://michaelgogins.tumblr.com
michael dot gogins at gmail dot com

## vst3edit

vst3-opcodes -- VST plugin hosting in Csound.

### Description

### Syntax

### Initialization

### Performance

### Examples

### See Also

### Credits

Michael Gogins 
http://michaelgogins.tumblr.com
michael dot gogins at gmail dot com

## vst3info

vst3-opcodes -- VST plugin hosting in Csound.

### Description

### Syntax

### Initialization

### Performance

### Examples

### See Also

### Credits

Michael Gogins 
http://michaelgogins.tumblr.com
michael dot gogins at gmail dot com

## vst3init

vst3-opcodes -- VST3 plugin hosting in Csound.

### Description

**vst3init** is used to load a VST3 plugin into memory for use with the 
other vst3-opcodes. Both VST3 effects and instruments (synthesizers) can be 
used. Note that for VST3, there may be multiple plugins defined in one 
loadable module.

### Syntax

i_handle **vst3init** S_module_filepath, S_plugin_name [,i_verbose]

### Initialization

*i_handle* -- the handle that identifies the plugin, to be passed to other 
vst3-opcodes.

*S_module_filepath* -- the full pathname of the vst plugin shared library (dll, 
on Windows). Remember to use '/' instead of '\' as the path separator.

*S_plugin_name* -- the name of the plugin within the module. If there is only 
one plugin, this can be an empty string.

*i_verbose* -- print plugin information when loading. This includes a list of all 
the plugins defined in the module.

### Examples

### See Also

[vst3audio](#vst3audio), [vst3bankload](#vst3bankload), 
[vst3banksave](#vst3banksave), [vst3edit](#vst3edit),
[vst3info](#vst3info), [vst3init](#vst3init),
[vst3midiout](#vst3midiout), [vst3note](#vst3note),
[vst3paramget](#vst3paramget), [vst3paramset](#vst3paramset),
[vst3progset](#vst3progset), [vts3tempo](#vts3tempo)

### Credits

Author:

Michael Gogins 
http://michaelgogins.tumblr.com
michael dot gogins at gmail dot com

## vst3midiout

vst3-opcodes -- VST plugin hosting in Csound.

### Description

### Syntax

### Initialization

### Performance

### Examples

### See Also

### Credits

Michael Gogins 
http://michaelgogins.tumblr.com
michael dot gogins at gmail dot com

## vst3note

### Description

### Syntax

### Initialization

### Performance

### Examples

### See Also

### Credits

Michael Gogins 
http://michaelgogins.tumblr.com
michael dot gogins at gmail dot com

## vst3paramget

vst3-opcodes -- VST plugin hosting in Csound.

### Description

### Syntax

### Initialization

### Performance

### Examples

### See Also

### Credits

Michael Gogins 
http://michaelgogins.tumblr.com
michael dot gogins at gmail dot com

## vst3paramset

### Description

### Syntax

### Initialization

### Performance

### Examples

### See Also

### Credits

Michael Gogins 
http://michaelgogins.tumblr.com
michael dot gogins at gmail dot com

## vst3progset

vst3-opcodes -- VST plugin hosting in Csound.

### Description

### Syntax

### Initialization

### Performance

### Examples

### See Also

### Credits

Michael Gogins 
http://michaelgogins.tumblr.com
michael dot gogins at gmail dot com

## vst3tempo

vst3-opcodes -- VST plugin hosting in Csound.

### Description

### Syntax

### Initialization

### Performance

### Examples

### See Also

[vst3audio](#vst3audio), [vst3bankload](#vst3bankload), 
[vst3banksave](#vst3banksave), [vst3edit](#vst3edit),
[vst3info](#vst3info), [vst3init](#vst3init),
[vst3midiout](#vst3midiout), [vst3note](#vst3note),
[vst3paramget](#vst3paramget), [vst3paramset](#vst3paramset),
[vst3progset](#vst3progset), [vts3tempo](#vts3tempo)

### Credits

Author:

Michael Gogins 
http://michaelgogins.tumblr.com
michael dot gogins at gmail dot com


## Description

Implements Jon Christopher Nelson's waveguide mesh reverb, originally created 
as a Cabbage VST plugin, as a C++ Csound plugin opcode. 

MVerb is a modified 5-by-5 2D waveguide mesh reverberator. It is highly 
flexible and can generate compelling and unique effects timbres ranging from 
traditional spaces to infinite morphing spaces or the simulation of metallic 
plates or cymbals. The plugin incorporates a 10-band parametric equalizer for 
timbral control and delay randomization to create more unusual effects.

## Syntax
```
aoutleft, aoutright MVerb ainleft, ainright, Spreset [[, Sparameter, kvalue ],...]
```
## Initialization

*Spreset* -- Name of a built-in preset, one of: "Small Hall", "Medium Hall", 
            "Large Hall", "Huge Hall", "Infinite Space", "Dry Echo", 
            "Right-Left", "Comby 1", "Comby 2", "Octaves", "TriTones", 
            "Big Dark", "Metallic 1", "Weird 1", "Weird 2", "Weird 3", 
            "Large Cymbal 1", "Large Cymbal 2", "Splash Cymbal 1", 
            "Splash Cymbal 2", "Turkish Cymbal", "Gong", "Small Gong", 
            "Metallic 2", "Tubular Metallic", "Cowbell", "Finger Cymbal", 
            "Bell", "Chinese Ball", "Cymbal Cap", "Baking Sheet", 
            "Frying Pan", "Squeak", "Trellace", "Monkey Wrench".

The order of initialization is:

1. All parameters have default values.
2. The user's choice of *Spreset* determines most of these parameters.
3. The user can override any number of default or preset parameters using 
   optional opcode parameters.

## Performance

*aoutleft* - Left channel of the output signal.

*aoutright* - Right channel of the output signal.

*ainleft* - Left channel of the input signal.

*ainright* - Right channel of the input signal.

*[[, Sparameter, xvalue ],...]* -- Any number of the following control 
                                   parameters, as *name, value* pairs. These 
                                   are real-valued unless they are strings.

- *wet* -- Fraction of the output signal that is reverberated. Not in a preset, must be set as an opcode parameter.
- *res1* -- Resonant frequency of node 1 in the mesh.
- *res2* -- Resonant frequency of node 2 in the mesh.
- *res3* -- Resonant frequency of node 3 in the mesh.
- *res4* -- Resonant frequency of node 4 in the mesh.
- *res5* -- Resonant frequency of node 5 in the mesh.
- *res6* -- Resonant frequency of node 6 in the mesh.
- *res7* -- Resonant frequency of node 7 in the mesh.
- *res8* -- Resonant frequency of node 8 in the mesh.
- *res9* -- Resonant frequency of node 9 in the mesh.
- *res10* -- Resonant frequency of node 10 in the mesh.
- *res11* -- Resonant frequency of node 11 in the mesh.
- *res12* -- Resonant frequency of node 12 in the mesh.
- *res13* -- Resonant frequency of node 13 in the mesh.
- *res14* -- Resonant frequency of node 14 in the mesh.
- *res15* -- Resonant frequency of node 15 in the mesh.
- *res16* -- Resonant frequency of node 16 in the mesh.
- *res17* -- Resonant frequency of node 17 in the mesh.
- *res18* -- Resonant frequency of node 18 in the mesh.
- *res19* -- Resonant frequency of node 19 in the mesh.
- *res20* -- Resonant frequency of node 20 in the mesh.
- *res21* -- Resonant frequency of node 21 in the mesh.
- *res22* -- Resonant frequency of node 22 in the mesh.
- *res23* -- Resonant frequency of node 23 in the mesh.
- *res24* -- Resonant frequency of node 24 in the mesh.
- *res25* -- Resonant frequency of node 25 in the mesh.
- *ERselect* -- Name of the early reflections preset, one of: "None", "Small", "Medium", "Large", "Huge", "Long Random", "Short Backwards", "Long Backwards", "Strange1", "Strange2".
- *ERamp* -- Amplitude of early reflections.
- *DFact* -- Amount (size) of early reflections.
- *FB* -- Delay feedback (size) of mesh.
- *FBclear* -- If true (1), clears all feedbacks in the mesh; if false (0), enables feedbacks.
- *Q* -- Q of the equalizer filters.
- *EQselect* -- Name of the equalization preset, one of: "flat", "high cut 1", "high cut 2", "low cut 1", "low cut 2", "band pass 1", "band pass 2", "2 bands", "3 bands", "evens", "odds".
- *eq1* -- Gain of equalizer band 1.
- *eq2* -- Gain of equalizer band 2.
- *eq3* -- Gain of equalizer band 3.
- *eq4* -- Gain of equalizer band 4.
- *eq5* -- Gain of equalizer band 5.
- *eq6* -- Gain of equalizer band 6.
- *eq7* -- Gain of equalizer band 7.
- *eq8* -- Gain of equalizer band 8.
- *eq9* -- Gain of equalizer band 9.
- *eq10* -- Gain of equalizer band 10.
- *random* -- Whether (1) or not (0) the randomization of mesh delays is enabled. Not in a preset, must be set as an opcode parameter.
- *rslow* -- Lower limit of frequency of randomization of mesh delay times. Not in a preset, must be set as an opcode parameter.
- *rfast* -- Upper limit of frequency of randomization of mesh delay times.  Not in a preset, must be set as an opcode parameter.
- *rmax* -- Maximum random deviation of mesh delay times. Not in a preset, must be set as an opcode parameter.
- *random_seed* -- Optional seed for the random number generator used by the delay time randomizers. If not set, the C++ library selects a seed. Not in a preset, must be set as an opcode parameter.
- *print* -- Optional flag to print all parameter values (at initialization time only) to the Csound message console. Not in a preset, must be set as an opcode parameter.
    
The order of processing is:

1.  2 DC blockers for the stereo input signal.
2.  2 multitap delays for stereo early reflections.
3.  25 mesh nodes for the reverb, each with:
    1.  4 variable delay lines, with optionally randomized delay times.
    1.  4 10-band equalizers, each with:    
        1.  10 parametric biquad filters.
        2.  1 level balancer.
        3.  1 DC blocker.        
4.  2 DC blockers for the stereo output signal.

## Credits

Jon Christopher Nelson wrote the original Cabbage VST plugin.

Michael Gogins adapted Nelson's plugin as a Csound plugin opcode in C++.
