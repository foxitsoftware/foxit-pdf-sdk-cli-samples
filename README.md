# foxit-pdf-sdk-cli-samples

**English** | [简体中文](README.zh-CN.md)

A collection of command-line examples for Foxit PDF SDK, showcasing common PDF
operations such as viewing, editing, conversion, OCR, signatures, and document
processing.

Each sample is a standalone console program that parses its own command-line
arguments. Run any sample with `--help` to print its full option list.

## Repository layout

```
cli_sample/
├── CMakeLists.txt              # CMake build script (Linux / macOS / ARM)
├── RunDemo.sh                  # Build & run helper for Linux / macOS
├── toolchain-armv7.cmake       # Cross-compile toolchain (ARMv7)
├── toolchain-armv8.cmake       # Cross-compile toolchain (ARMv8 / aarch64)
├── simple_demo_vs2022.sln      # Visual Studio 2022 solution (Windows)
└── <sample>/                   # One folder per sample
    ├── <sample>.cpp            # Sample source
    └── <sample>_vs20xx.vcxproj # Visual Studio project (2010/2015/2017/2019/2022)
```

The samples expect the Foxit PDF SDK to be located **two levels above this
folder**, i.e.:

```
<SDK_ROOT>/
├── include/                    # SDK headers
├── lib/                        # SDK libraries (fsdk_win64.lib, libfsdk_linux64.so, ...)
└── samples/cli_sample/         # this repository
```

## Prerequisites

- **Windows**: Visual Studio 2010/2015/2017/2019/2022 (matching the `.vcxproj`
  you open) and the Foxit PDF SDK for Windows.
- **Linux / macOS**: CMake 2.8+, `make`, and a C++11 compiler.
- **ARM**: a cross toolchain, e.g. `gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf`
  for ARMv7 or `gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu` for ARMv8. The
  toolchain path can be overridden with the `TOOLSCHAIN_ARMV7_PATH` /
  `TOOLSCHAIN_ARMV8_PATH` environment variable.

> **License**: every sample initializes the SDK with an empty serial number and
> key (`static const char* sn = ""; static const char* key = "";`). Replace these
> with your own license before running, otherwise `Library::Initialize` fails.

## Build & run

### Windows (Visual Studio)

1. Open `simple_demo_vs2022.sln` (or the `.vcxproj` of a single sample).
2. Select the configuration (`Debug`/`Release`, `Win32`/`x64`) and build.
3. Executables are written to `bin/` with a configuration suffix, e.g.
   `bin/annotation_rel_x64_vs2022.exe`.

### Linux / macOS (CMake + RunDemo.sh)

`RunDemo.sh` builds and immediately runs one or more samples:

```bash
./RunDemo.sh annotation                 # build & run a single sample
./RunDemo.sh annotation pdf2text        # build & run several samples
./RunDemo.sh all                        # build & run every sample
./RunDemo.sh annotation dbg             # debug build
```

The produced binary is named `<sample>_<os><arch>`, e.g. `annotation_linux64`,
`pdf2text_mac64`, `ocr_linuxarmv8`.

### Manual CMake build

```bash
cmake . -DPRJ_NAME=pdf2text
make
./pdf2text_linux64 --help
```

### ARM cross-compile

```bash
cmake . -DPRJ_NAME=pdf2text -DCMAKE_TOOLCHAIN_FILE=./toolchain-armv7.cmake
make
```

## Command-line conventions

Most samples follow a common convention:

- `-i, --input <path>` — input file (PDF, image, DWG, text, ...).
- `-o, --output <path>` — output file or directory.
- `--action <type>` / `--op <type>` — the operation to perform.
- `--help` — print usage and exit.

A few older samples use positional arguments instead (see the table below).
Always run `<sample> --help` for the authoritative, up-to-date option list.

## Sample reference

### Viewing & rendering


| Sample           | Command                                                                                         |
| ------------------ | ------------------------------------------------------------------------------------------------- |
| `pdf2image`      | `pdf2image --input <in.pdf> --output-dir <dir> [--pages all|1,3-5] [--formats bmp,jpg,png,...]` |
| `pdfreflow`      | `pdfreflow --input <in.pdf> --output <dir> [--mode single|continuous|both]`                     |
| `output_preview` | `output_preview --input <in.pdf> --output <out.bmp> --icc <icc profile>`                        |
| `async_load`     | `async_load_xxx -i <in.pdf> -o <out.bmp>`                                                       |
| `pdfprint`       | `pdfprint --input <in.pdf> [--printer <name> | --output <dir>] [--start-page n] [--end-page n]` |
| `pdfwrapper`     | `pdfwrapper --action <wrap|payload|info|extract> --input <in.pdf> --output <out>`               |

### Conversion


| Sample       | Command                                                                             |
| -------------- | ------------------------------------------------------------------------------------- |
| `pdf2text`   | `pdf2text --input <in.pdf> --output <out.txt>`                                      |
| `pdf2xml`    | `pdf2xml --input <in.pdf> --output <out.xml> [--image-folder <dir>]`                |
| `pdf2office` | `pdf2office --input <in.pdf> --output <out.docx> [--format word|excel|ppt]`         |
| `office2pdf` | `office2pdf_xxx --input <office file> --output <out.pdf> --type <word|excel|ppt>`   |
| `html2pdf`   | `html2pdf_xxx -html <url or html path> -o <out.pdf>`                                |
| `image2pdf`  | `image2pdf_xxx -o <out.pdf> -p <page>:<x>:<y>:<image> [-p ...]`                     |
| `text2pdf`   | `text2pdf --input <in.txt> --output <out.pdf> [--font-name <name>] [--text-size n]` |
| `dwg2pdf`    | `dwg2pdf_xxx -i <in.dwg> -o <out.pdf> --engine <path>`                              |

### Editing & page operations


| Sample               | Command                                                                                          |
| ---------------------- | -------------------------------------------------------------------------------------------------- |
| `annotation`         | `annotation_xxx [--op add|delete|edit] [-i <in.pdf>] [-o <out.pdf>] [-p <page>] [--type <type>]` |
| `annotation_summary` | `annotation_summary -i <in.pdf> -o <out.pdf> [--layout 0-4]`                                     |
| `bookmark`           | `demo_bookmark --action <get|add> -i <in.pdf> [options]`                                         |
| `page_organization`  | `page_organization --input-dir <dir> --output <out.pdf> --ops <insert,remove,import,move>`       |
| `page_labels`        | `page_labels --op <create|delete|update|query> --input <in.pdf> [options]`                       |
| `pdflayer`           | `pdflayer --action <query|add|modify|remove|move> --input <in.pdf> --output <out.pdf>`           |
| `pdfheaderfooter`    | `pdfheaderfooter --action <add|update|remove> --input <in.pdf> --output <out.pdf>`               |
| `watermark`          | `demo_watermark --action <add|remove> --input <in.pdf> --output <out.pdf>`                       |
| `graphics_objects`   | `graphics_objects -i <in.pdf> -o <out.pdf> --op <add|remove> --object-types <text|image|path>`   |
| `matrix`             | `matrix <in.pdf> <out.pdf> --op <translate|rotate|scale|shear> [params]`                         |
| `paragraph_editing`  | `paragraph_editing --input <in.pdf> --output-dir <dir> [options]`                                |
| `marked_content`     | `marked_content <in.pdf> <out.pdf>`                                                              |
| `pdfredact`          | `pdfredact --input <in.pdf> --output <out.pdf> --text <pattern>`                                 |
| `searchreplace`      | `searchreplace --input <in.pdf> --output <out.pdf> --pattern <text> --replacement <text>`        |
| `pdfflatten`         | `pdfflatten --input <in.pdf> --output <out.pdf> [--option all|no-annot|no-form]`                 |

### Forms


| Sample             | Command                                                                                                         |
| -------------------- | ----------------------------------------------------------------------------------------------------------------- |
| `form`             | `demo_form -i <in.pdf> -o <out.pdf> --field-types <pushbutton,radiobutton,checkbox,textfield,listbox,combobox>` |
| `form_combination` | `demo_form_combination -i <in1> [in2 ...] -o <out.csv>`                                                         |
| `form_recognition` | `demo_form_recognition -i <in.pdf> -o <out.pdf>`                                                                |
| `fillsign`         | `demo_fillsign -i <in.pdf> -o <out.pdf> --object-types <crossmark,checkmark,roundrect,line,dot,text,signature>` |
| `fdf`              | `fdf -i <in.pdf> -o <out> --mode <pdf2fdf|pdf2xfdf|fdf2pdf|xfdf2pdf> --type <annot|form|annot,form>`            |
| `xfa_form`         | `xfa_form --input <in.pdf> --output <dir> --action <export|import|reset>`                                       |
| `electronictable`  | `demo_electronictable --output <out.pdf> --data <data.json>`                                                    |

### Security & signatures


| Sample                  | Command                                                                                       |
| ------------------------- | ----------------------------------------------------------------------------------------------- |
| `security`              | `demo_security --action <std|certificate|drm|custom|rms|all> --input <in.pdf> --output <dir>` |
| `signature`             | `demo_signature --action <sign|verify> --input <in.pdf> --output <dir> --cert-path <pfx>`     |
| `pades`                 | `pades --input <in.pdf> --output <out.pdf> --cert <cert.pfx> --cert-password <pwd>`           |
| `ltv`                   | `ltv -i <in.pdf> -o <out.pdf> --cert <cert> --cert-password <pwd> [--digest sha256]`          |
| `paging_seal_signature` | `paging_seal_signature --input <in.pdf> --output <out.pdf> [options]`                         |

### Search


| Sample           | Command                                                                      |
| ------------------ | ------------------------------------------------------------------------------ |
| `search`         | `search --input <in.pdf> --output <out.txt> --pattern <text> [--match-case]` |
| `fulltextsearch` | `demo_fulltextsearch -i <in folder> -o <out folder> [options]`               |

### OCR & recognition


| Sample               | Command                                                                                     |
| ---------------------- | --------------------------------------------------------------------------------------------- |
| `ocr`                | `ocr -type <0|1|2> -input <in.pdf> -output <out> -engine <ocr engine path> [-lang <langs>]` |
| `layout_recognition` | `layout_recognition <in.pdf> <out info path>`                                               |
| `barcode`            | `demo_barcode --type <code39|code128|ean8|upca|ean13|itf|pdf417|qrcode> -o <out image>`     |
| `bitmap_transform`   | `demo_bitmap_transform --op <operation> -i <in bitmap> -o <out bitmap>`                     |

### Document processing


| Sample             | Command                                                                                                              |
| -------------------- | ---------------------------------------------------------------------------------------------------------------------- |
| `pdfcombination`   | `pdfcombination --input <a.pdf> [--bookmark <title>] ... --output <out.pdf>`                                         |
| `pdfcompare`       | `pdfcompare --base <base.pdf> --compared <cmp.pdf> --output <dir>`                                                   |
| `compliance`       | `compliance -i <in.pdf> -o <out.pdf> --type <PDFA|PDFX|PDFE> --resource-dir <path>`                                  |
| `preflight`        | `preflight --action <list|analyze|fixup> [--input <in.pdf>] [--key <preflight key>]`                                 |
| `optimization`     | `optimization --input <in.pdf> --output <out.pdf> --options <bitmask>`                                               |
| `portfolio`        | `portfolio --action <create|info> --input <path> --output <path>`                                                    |
| `attachment`       | `attachment_xxx --action <get|add|delete|edit> -i <in.pdf> -o <out> [--key <name>] [--attach <file>]`                |
| `associated_files` | `associated_files_xxx -i <in.pdf> -o <out> --action <get|add|delete|edit> --command <catalog|page|annot|image|form>` |
| `docinfo`          | `docinfo -i <in.pdf> -o <out.txt> [--mode metadata|view|both]`                                                       |
| `psi`              | `psi --output <dir> --points-file <points.txt> [--diameter n] [--color 0xRRGGBB]`                                    |
| `taggedpdf`        | `taggedpdf --input <in.pdf> --output <out.pdf> [--callback]`                                                         |

## Notes

- Samples that need extra runtime resources (OCR engine, DWG engine, compliance
  resource folder, ICC profile, certificates) take the corresponding path as a
  command-line option — see each sample's `--help`.
- On macOS, `RunDemo.sh` exports `LD_LIBRARY_PATH` from `DWG_ENGINE_PATH` for the
  `dwg2pdf` sample.
- The `signature`, `security`, and `paging_seal_signature` samples additionally
  link against `libssl.a` / `libcrypto.a`.
