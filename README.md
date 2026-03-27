# HIT (HRT Imaging Tool)

## License

The source code of this project is licensed under the GNU General Public License v2.0 (GPL 2.0).

Copyright: Karlsruhe Institute of Technology (KIT)

The example image data in `/img_data/` is licensed under the Creative Commons Attribution 4.0 International license (CC BY 4.0) and includes its own license file. Please check the subdirectory for details.

## Installation

_HIT_ does not require any installation, you just run the executable HITgui.exe, that's all. There are two ways to obtain the executable:

- Download the latest release version from the [Releases](https://gitlab.kit.edu/kit/iai/ml4time/cornea/hit/-/releases) page
- Get the source code and compile it yourself (see below for further instructions)

## Description

This project provides algorithms to create mosaic images (i.e. mappings or wide-field montages) from image datasets acquired with the _HRT_ (a commercial laser-scanning confocal microscope for ophthalmological imaging; Heidelberg Engineering GmbH, Heidelberg, Germany). The algorithms have been developed for datasets of corneal tissue which are imaged using the _RCM_ (a commercial objective module for the _HRT_ system; Heidelberg Engineering GmbH, Heidelberg, Germany) or the [_RCM2.0_](https://doi.org/10.1364/BOE.9.002511) (an experimental research device to replace the _RCM_, developed at the University of Rostock).

The datasets can either be continuous consecutive _HRT_ image sequences (which in this context means _HRT_ image sequences recorded with 30 frames per second) such as sequences recorded in Sequence Scan Mode with 30 Hz or using the Streaming Mode. This often involves guided eye movements in the imaging process, for example with the [_EyeGuidance_ system](https://doi.org/10.1515/cdbme-2016-0096) (an experimental research device, developed at KIT). Descriptions of this usage can be found in [1-4]. Alternatively, __HIT__ can also process datasets consisting of arbitrary collections of _HRT_ images (for example recorded using the Sequence Scan Mode with less than 30 Hz or repeatedly using the Section Scan Mode). Examples of this usage can be found in [5-8].

In addition to the source code, this repository also offers the option to download precompiled binaries for convenient usage. __HIT__ does not need installation, you can simply start the (downloaded or compiled) executable `HITgui.exe`. This opens a simple graphical user interface, which let's you add or remove datasets to be processed and finally start the process.

If you use __HIT__ for the first time, you should open the process parameter dialog once (using the `-p` switch from the command line, see the _Advanced_ section below) and choose one of the process parameter presets available via the buttons at the very top. If you are unsure which one is the correct one for your purpose, then chances are good that it's the first one, titled "Defaults for 2D datasets without oscillation".

## Dataset preparation

__HIT__ requires datasets in the form of multi-page TIFF files. Since the _HRT_ doesn't offer such an export option, you need to create the dataset files yourself. There are plenty of tools that can create multi-page TIFF files from a given set of single-page TIFF files (or other image formats). One option is the free image viewer software [_IrfanView_](https://www.irfanview.com/), which offers either a graphical user interface or alternatively also a command line interface for this. On the command line, you can create a multi-page TIFF file as follows:

`i_view64.exe /cmdexit /multitif=("C:\path\to\outputfile.tif","C:\path\to\inputfolder\*.tif")`

## Example Data

The subdirectory `/imgdata/` contains an example dataset file `01_LE.tif`. The image data are from a public repository [9], the dataset file has been created as described above. The results have been created with __HIT__ using the default process parameters (which can be reset by clicking the button "Defaults for 2D datasets" in the parameter dialog). To reproduce the results,

1. make sure that the default process parameters are set (open the parameter dialog window as described below, press the button titled "Defaults for 2D datasets", and exit with "OK"),
2. start __HIT__,
3. add the dataset file, and
4. press "Start".

This will create a subfolder named `01_LE` next to the dataset file, which contains the result files.

## Advanced

__HIT__ knows two switches that you can only use on the command line:

- `HITgui.exe -p` will open another dialog window that provides you with access to all process parameters. The parameter settings are stored in an SQLite database file which resides under `%APPDATA%\KIT-IAI\HIT.sqlite`. If you know what you're doing, you can also change the parameter settings directly in the database file.
- `HITgui.exe -a "C:\path\to\datasetfile.tif"` will open the __HIT__ GUI, process the given dataset file, and automatically close __HIT__ again when finished. This option can be used for automated workflows.

## Requirements for Compiling

If you prefer to compile the source code yourself, there are a few prerequisites to be fulfilled. This project includes build files for _MS Visual Studio 2022_; this is the only IDE and build system that is explicitly supported and has been tested. The following description assumes the usage of _MS Visual Studio 2022_.

The following features have to be installed in _Visual Studio_:

- Desktop Development with C++ (under "Workloads")
- C++ MFC for latest v143 build tools (x86 & x64) (under "Individual Components")
- vcpkg (under "Individual Components"; this should already be selected)
- English (under "Language Packs"; if not already selected)

Before compilation, open a terminal window from within _Visual Studio_ and execute the following command to activate vcpkg integration in the IDE:

`vcpkg integrate install`

## Authors

- [Stephan Allgeier](https://www.iai.kit.edu/english/2154_2149.php) (email: stephan.allgeier (at) kit.edu)
- [Klaus-Martin Reichert](https://www.iai.kit.edu/english/2154_2422.php)
- [Ekaterina Korn](https://www.iai.kit.edu/english/2154_4363.php) (geb. Sorokovaia)
- Many, many former colleagues and students (please contact me (Stephan Allgeier) if you want to be named here, I will gladly add any of you with your approval)

## Literature

[1] Allgeier S, Bartschat A, Bohn S, Guthoff RF, Hagenmeyer V, Kornelius L, Mikut R, Reichert KM, Sperlich K, Stache N, Stachs O, Köhler B. "Real-time large-area imaging of the corneal subbasal nerve plexus". Sci Rep. 2022;12:2481. doi: [10.1038/s41598-022-05983-5](https://doi.org/10.1038/s41598-022-05983-5).

[2] Allgeier S, Bartschat A, Bohn S, Peschel S, Reichert KM, Sperlich K, Walckling M, Hagenmeyer V, Mikut R, Stachs O, Köhler B. "3D confocal laser-scanning microscopy for large-area imaging of the corneal subbasal nerve plexus". Sci Rep. 2018;8:7468. doi: [10.1038/s41598-018-25915-6](https://doi.org/10.1038/s41598-018-25915-6).

[3] Allgeier S, Winter K, Bretthauer G, Guthoff RF, Peschel S, Reichert KM, Stachs O, Köhler B. "A novel approach to analyze the progression of measured corneal sub-basal nerve fiber length in continuously expanding mosaic images". Curr Eye Res. 2017;42(4):549-556. doi: [10.1080/02713683.2016.1221977](https://doi.org/10.1080/02713683.2016.1221977).

[4] Allgeier S, Maier S, Mikut R, Peschel S, Reichert KM, Stachs O, Köhler B. "Mosaicking the subbasal nerve plexus by guided eye movements". Invest Ophthalmol Vis Sci. 2014;55(9):6082-6089. doi: [10.1167/iovs.14-14698](https://doi.org/10.1167/iovs.14-14698).

[5] Andréasson M, Lagali N, Badian RA, Utheim TP, Scarpa F, Colonna A, Allgeier S, Bartschat A, Köhler B, Mikut R, Reichert KM, Solders G, Samuelsson K, Zetterberg H, Blennow K, Svenningsson P. "Parkinson's disease with restless legs syndrome-an in vivo corneal confocal microscopy study". NPJ Parkinsons Dis. 2021;7:4. doi: [10.1038/s41531-020-00148-5](https://doi.org/10.1038/s41531-020-00148-5).

[6] Badian RA, Allgeier S, Scarpa F, Andréasson M, Bartschat A, Mikut R, Colonna A, Bellisario M, Utheim TP, Köhler B, Svenningsson P, Lagali N. "Wide-field mosaics of the corneal subbasal nerve plexus in Parkinson's disease using in vivo confocal microscopy". Sci Data. 2021;8:306. doi: [10.1038/s41597-021-01087-3](https://doi.org/10.1038/s41597-021-01087-3).

[7] Lagali NS, Allgeier S, Guimarães P, Badian RA, Ruggeri A, Köhler B, Utheim TP, Peebo B, Peterson M, Dahlin LB, Rolandsson O. "Wide-field corneal subbasal nerve plexus mosaics in age-controlled healthy and type 2 diabetes populations". Sci Data. 2018;5:180075. doi: [10.1038/sdata.2018.75](https://doi.org/10.1038/sdata.2018.75).

[8] Lagali NS, Allgeier S, Guimarães P, Badian RA, Ruggeri A, Köhler B, Utheim TP, Peebo B, Peterson M, Dahlin LB, Rolandsson O. "Reduced corneal nerve fiber density in type 2 diabetes by wide-area mosaic analysis". Invest Ophthalmol Vis Sci. 2017;58(14):6318-6327. doi: [10.1167/iovs.17-22257](https://doi.org/10.1167/iovs.17-22257).

[9] Badian RA, Lagali N, Allgeier S, Scarpa F, Andreasson M, Bartschat A, Mikut R, Colona A, Bellisario M, Paaske Utheim T, Köhler B, Svenningsson P. "Wide-field mosaic dataset of the corneal subbasal nerve plexus in Parkinson’s disease using in vivo confocal microscopy". figshare. 2021. doi: [10.6084/m9.figshare.14481249.v2](https://doi.org/10.6084/m9.figshare.14481249.v2).
