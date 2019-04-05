SNES Loader for IDA
===================

This is a IDA 6.x loader plugin for Super Nintendo Entertainment System / Super Famicom (SNES / SFC) ROM (*.smc; *.sfc)

With [65816 processor plugin](https://github.com/gocha/ida-65816-module), you can load a SNES cartridge file into IDA.

**IMPORTANT NOTICE**:
The SNES loader was forked from [IDA SDK 6.8](https://www.hex-rays.com/products/ida/support/download.shtml),
published by [Hex-Rays](https://www.hex-rays.com/). In IDA 7.0, Hey-Rays has made breaking changes on IDA API design and has been released an [migrating guide](https://www.hex-rays.com/products/ida/7.0/docs/api70_porting_guide.shtml). I don't plan to maintain this repository to follow new IDA API design at the moment. I think I will try to make an IDAPython plugin from scratch instead, when I seriously need an extension for SNES reverse engineering, but I really don't need that very soon.

**To IDA 7.x users**: You can find the source code in the official IDA SDK.

How to compile
--------------

1. Download and install [IDA SDK](https://www.hex-rays.com/products/ida/support/download.shtml) (expected version is IDA SDK 6.9)
2. Clone the repository into $(IDASDK)/ldr/snes
3. Compile the project with [Visual Studio](https://www.visualstudio.com/downloads/download-visual-studio-vs.aspx)

Read official development guides for more details of generic IDA development.
