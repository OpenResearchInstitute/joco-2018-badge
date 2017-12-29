# File formats supported #

* BMP 24-bit RGB uncompressed	 	Very slow
* BMP 16-bit 565 RGB uncompressed  	Slow
* RAW 16-bit 565 Big Endian uncompressed 	Fast		NOTE: Must be 128x128

# Commands #

16-bit RGB Big Endian Raw file (very fast)
```
ffmpeg -i RED.BMP -f image2 -s 128x128 -pix_fmt rgb565be RED.raw
```

Convert animated gif to raw video format:
```
ffmpeg -i MEATSPIN.gif -r 22 -f rawvideo -s 128x128 -pix_fmt rgb565be MEATSPIN.RAW
```

Convert image for a menu icon:
```
ffmpeg -i ICON.png -f image2 -s 20x20 -pix_fmt rgb565be ICON.RAW
mv WRENCH.RAW WRENCH.ICO
```

Convert image for preview:
```
ffmpeg -i PREVIEW.jpg -f image2 -s 101x112 -pix_fmt rgb565be PREVIEW.RAW
mv BADGERS.RAW BADGERS.PRV
```

Convert animated gif to RGB file for LED animation:
```
ffmpeg -i flames-led.gif -r 20 -f rawvideo -s 5x4 -pix_fmt rgb24 FLAMES.RGB
```
