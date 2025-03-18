# SSSH
The Sega Saturn Smart Home Project<br>
from bl3i<br><br>

Here contains the code for the Sega Saturn Smart Home v0.4.5.<br>
Watch the YouTube video @ https://youtu.be/o6kFTAdcRiw<br><br>

The code (main.c above) is intended for use with the Jo Engine library, found @ https://www.jo-engine.org/home/<br>
You can download the .iso (featuring my graphics and audio) and run it yourself @ https://github.com/hozkinz/SSSH/releases/tag/public or https://bl3i.com/sssh<br>
How to hack/integrate it with an Alexa is also detailed on that page, as is my personal blog on how and why I did this.<br><br>

If building yourself, images have to be a division of 8 pixels wide. Filenames also have to be maximum 8 characters long (I believe/forget, long filenames never worked anyway).<br>
.PCM files are encoded via ffmpeg @ 22050hz, which is different from the code, but it was the only way I could get a decent speed. Other times, it would be too slow/fast (on console).<br>
The easiest way to use this in a custom way would be to copy the "demo - audio" folder in the Jo Engine samples folder and replace the graphics/sound with anything you want.<br><br>

The internet is WRONG about being able to build your own Alexa using a Raspberry Pi. Unless I find out differently, the AVS service that many instruction sheets use was shut off late last year (2024). I couldn't find out how regardless, although I know Open Source solutions are already available.<br><br>

Code is open source/do what you like. I wouldn't mind a shout-out though for any of your further developments, or keeping me apprised of any of your own modifications.<br>
