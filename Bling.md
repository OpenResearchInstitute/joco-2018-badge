# How Bling works

There are two (well, three) categories of bling.

One I’ll call ‘regular’ bling, and that’s a mode that appears in the bling menus on the badge.

The second is a “backer’ bling, and that’s basically the same except it’s in a different menu location.

The third is “BYOB” or “Bring your own Bling”, which can be added by just copying files to the SD card. Those files appear in the menu under a ‘custom bling’ mode and can be launched from there.

The file formats are the same for all these modes and you can test bling by using BYOB before adding it to the menus, which requires source code changes to the badge code base. There’s a google doc titled “Menu Layout” in the same folder as this doc that has the Bender badge menu hierarchy.

In addition to the gif file with the animation for the LCD (.RAW extension), each bling mode ~may~ also have a custom callback associated with it in the menu which defines the LED flashing patterns. If there’s not a custom callback, one is used that maps the LED flashing and colors to the frames in the gif file. I don’t completely understand this yet, I haven’t looked at it much. I do know that we’ll need to change the mapping because of the differences between the Bender eye LEDs and the pirate monkey tooth.

And lastly, there are “.RGB” files, which is a 4x3 sized file in the same format, to be applied to the LEDs along with the BYOB modes in “custom bling”.

[This page](https://hackaday.io/project/19121-andxor-dc25-badge/log/58327-secret-component-feature-sd-card-byob) describes how to prep the gif files and place them on the SD card for BYOB.


Menu entries also need an icon (.ICO) and a preview (.PRV) file in addition to the .RAW file.

## Additional notes for bling selected from menus

File Format(s)
The RAW, PRV, and ICO files are all in rawvideo rgb565be format.

PRV and ICO files only contain a single frame

RAW files are 128 x 128 pixels
PRV files are 101 x 112 pixels
ICO files are 20 x 20 pixels and all the current ones are two-color (black and white)

## File Locations on the SD Card

Typically, RAW files are in the “BLING” directory

Typically, PRV and ICO files are in the “MENU” directory

## Workflow for preparing all the files for a bling mode

This assumes that you want to create the RAW file and preview from an animated gif, and the icon from a separate file. There are shell scripts in the joco repository to make it easier to use ffmpeg to do the conversions. These are in the imagetools directory.

* Start with an animated gif, which must be at least 128x128 pixels.

* Crop it using this tool. Create one file that’s 128x128 and one that’s 101x112

### Generate the RAW file

```./gif2raw input-128x128.gif outfile.RAW
```

### Generate the PRV file (uses the first frame of the gif for the preview)

```./gif2prv input-101x112.gif outfile.PRV
```

### Generate the ICO file

Create a 20x20 pixel icon using the tools of your choice. For consistency, make it black and white. Export/save it as a gif file.

```./gif2prv input-20x20.gif outfile.ICO
```

