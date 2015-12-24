SNES Loader for IDA
===================

This is a IDA loader plugin for Super Nintendo Entertainment System / Super Famicom (SNES / SFC) ROM (*.smc; *.sfc)

With [65816 processor plugin](https://github.com/gocha/ida-65816-module), you can load a SNES cartridge file into IDA.

**NOTICE**:
The SNES loader was initially forked from [IDA SDK 6.8](https://www.hex-rays.com/products/ida/support/download.shtml),
published by [Hex-Rays](https://www.hex-rays.com/).

How to compile
--------------

1. Download and install [IDA SDK](https://www.hex-rays.com/products/ida/support/download.shtml) (expected version is IDA SDK 6.9)
2. Clone the repository into $(IDASDK)/ldr/snes
3. Compile the project with [Visual Studio](https://www.visualstudio.com/downloads/download-visual-studio-vs.aspx)

Read official development guides for more details of generic IDA development.
