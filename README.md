# SpectralAnalyzer

A visualization framework for sound recorded from the microphone. Three different visualization styles are currently available:
* an oscilloscope,
* a spectral intensity plot, showing sound intensity as a function of frequency,
* a sliding window spectrogram, with time on the horizontal axis, frequency on the vertical axis, and intensity encoded with color.

There are a number of keys (currently hard-coded) that flip between the displays and change their parameters:

## Keyboard controls

### GENERAL
Command                   |   Keyboard shortcut
--------------------------|---------------------
Exit program:             |   `Escape`
Next display type:        |   `d`
Previous display type:    |   `SHIFT + d ('D')`
Next input source:        |   `i`
Previous input source:    |   `SHIFT + i ('I')`

### SPECTROGRAM
Command                     |   Keyboard shortcut
----------------------------|---------------------
Reset axes:                 |  `r`
Log scaling for frequencies:|  `l`
Log scaling for amplitudes: |  `SHIFT + l ('L')`
Zoom in frequency domain:   |  `=`
Zoom out frequency domain:  |  `-`
Shift frequency domain up:  |  `UP`
Shift freq. domain down:    |  `DOWN`
Increase gain:              |  `SHIFT + = ('+')`
Decrease gain:              |  `SHIFT + - ('_')`
Move amplitude range up:    |  `RIGHT`
Move amplitude range down:  |  `LEFT`

### SPECTRAL ENVELOPE
Command                     |   Keyboard shortcut
----------------------------|---------------------
Reset axes:                 |  `r`
Flip axes visibility:       |  `a`
Flip grid visibility:       |  `g`
Log scaling for frequencies:|  `SHIFT + l ('L')`
Log scaling for amplitudes: |  `l`
Zoom in frequency domain:   |  `SHIFT + = ('+')`
Zoom out frequency domain:  |  `SHIFT + - ('_')`
Shift frequency domain up:  |  `RIGHT`
Shift freq. domain down:    |  `LEFT`
Increase gain:              |  `=`
Decrease gain:              |  `-`
Move amplitude range up:    |  `UP`
Move amplitude range down:  |  `DOWN`

OSCILLOSCOPE
============
Command                     |   Keyboard shortcut
----------------------------|---------------------
Reset axes:                 |   `r`
Flip axes visibility:       |   `a`
Flip grid visibility:       |   `g`
Next display style:         |   `f`
Previous display style:     |   `SHIFT + f ('F')`
Flip starting at zero cross:|   `s`
Zoom in amplitude:          |   `=`
Zoom out amplitude:         |   `-`

Display style cycle is: (lines only, lines and dots, dots only)
