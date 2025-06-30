# HIT (HRT Imaging Tool)

## Description

This software application provides algorithms to create mosaic images (i.e. mappings or wide-field montages) from image datasets acquired with the HRT (a commercial laser-scanning confocal microscope for ophthalmological imaging; Heidelberg Engineering GmbH, Heidelberg, Germany). The algorithms have been developed for datasets of corneal tissue which are imaged using the RCM (a commercial objective module for the HRT system; Heidelberg Engineering GmbH, Heidelberg, Germany) or the RCM2.0 (an [experimental research device](https://doi.org/10.1364/BOE.9.002511) [1] to replace the RCM, developed at the University of Rostock).

The datasets can either be continuous consecutive HRT image sequences --- which in this context means HRT image sequences recorded with 30 frames per second --- such as sequences recorded in Sequence Scan Mode with 30 Hz or using the Streaming Mode. A description of this usage can be found in [2]. Alternatively, HIT can also process datasets consisting of arbitrary collections of HRT images (for example recorded using the Sequence Scan Mode with less than 30 Hz or repeatedly using the Section Scan Mode). An example of this usage can be found in [3].

HIT does not need installation, you can simply start the compiled executable `HITgui.exe`. This opens a simple graphical user interface, which let's you add or remove datasets to be processed and finally start the process.

## Dataset preparation

HIT requires datasets in the form of multi-page TIFF files. Since the HRT doesn't offer such an export option, you need to create the dataset files yourself. There are plenty of tools that can create multi-page TIFF files from a given set of single-page TIFF files (or other image formats). One option is the free image viewer software [IrfanView](https://www.irfanview.com/), which offers either a graphical user interface or alternatively also a command line interface for this. On the command line, you can create a multi-page TIFF file as follows:

'i_view64.exe /cmdexit /multitif=("C:\path\to\outputfile.tif","C:\path\to\inputfolder\*.tif")'

## Advanced

HIT knows two switches that you can only use on the command line:

- `HITgui.exe -p` will open another dialog window that provides you with access to all process parameters. The parameter settings are stored in an SQLite database file which resides under `%APPDATA%\KIT-IAI\HIT.sqlite`. If you know what you're doing, you can also change the parameter settings directly in the database file.

- `HITgui.exe -a "C:\path\to\datasetfile.tif"` will open the HIT GUI, process the given dataset file, and automatically close HIT again when finished. This option can be used for automated workflows.

## Installation

TBD

## Authors

- [Stephan Allgeier](https://www.iai.kit.edu/english/2154_2149.php)
- [Klaus-Martin Reichert](https://www.iai.kit.edu/english/2154_2422.php)
- Many, many former colleagues and students (please contact me (Stephan Allgeier) if you want to be named here, I will gladly add any of you with your approval)

## Literature

[1] Bohn S, Sperlich K, Allgeier S, Bartschat A, Prakasam R, Reichert KM, Stolz H, Guthoff R, Mikut R, Köhler B, Stachs O, "Cellular in vivo 3D imaging of the cornea by confocal laser scanning microscopy". Biomed Opt Express 2018;9(6):2511-2525. doi: [10.1364/BOE.9.002511](https://doi.org/10.1364/BOE.9.002511).

[2] Allgeier S, Bartschat A, Bohn S, Guthoff RF, Hagenmeyer V, Kornelius L, Mikut R, Reichert KM, Sperlich K, Stache N, Stachs O, Köhler B. "Real-time large-area imaging of the corneal subbasal nerve plexus". Sci Rep. 2022;12:2481. doi: [10.1038/s41598-022-05983-5](https://doi.org/10.1038/s41598-022-05983-5).

[3] Badian RA, Allgeier S, Scarpa F, Andréasson M, Bartschat A, Mikut R, Colonna A, Bellisario M, Utheim TP, Köhler B, Svenningsson P, Lagali N. "Wide-field mosaics of the corneal subbasal nerve plexus in Parkinson's disease using in vivo confocal microscopy". Sci Data. 2021;8:306. doi: [10.1038/s41597-021-01087-3](https://doi.org/10.1038/s41597-021-01087-3).
