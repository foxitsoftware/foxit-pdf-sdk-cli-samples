# foxit-pdf-sdk-cli-samples

[English](README.md) | **简体中文**

本仓库是 Foxit PDF SDK 的命令行示例集合，涵盖查看、编辑、转换、OCR、签名、文档处理等常见 PDF 操作。

每个示例都是一个独立的控制台程序，自行解析命令行参数。运行任意示例时加上 `--help` 即可打印其完整选项列表。

## 仓库结构

```
cli_sample/
├── CMakeLists.txt              # CMake 构建脚本（Linux / macOS / ARM）
├── RunDemo.sh                  # Linux / macOS 的构建与运行辅助脚本
├── toolchain-armv7.cmake       # 交叉编译工具链（ARMv7）
├── toolchain-armv8.cmake       # 交叉编译工具链（ARMv8 / aarch64）
├── simple_demo_vs2022.sln      # Visual Studio 2022 解决方案（Windows）
└── <sample>/                   # 每个示例一个目录
    ├── <sample>.cpp            # 示例源码
    └── <sample>_vs20xx.vcxproj # Visual Studio 工程（2010/2015/2017/2019/2022）
```

示例要求 Foxit PDF SDK 位于**本目录的上两级**，即：

```
<SDK_ROOT>/
├── include/                    # SDK 头文件
├── lib/                        # SDK 库文件（fsdk_win64.lib、libfsdk_linux64.so 等）
└── samples/cli_sample/         # 本仓库
```

## 前置条件

- **Windows**：Visual Studio 2010/2015/2017/2019/2022（需与所打开的 `.vcxproj` 版本匹配）以及 Windows 版 Foxit PDF SDK。
- **Linux / macOS**：CMake 2.8+、`make` 以及支持 C++11 的编译器。
- **ARM**：交叉工具链，例如 ARMv7 使用 `gcc-arm-8.3-2019.03-x86_64-arm-linux-gnueabihf`，ARMv8 使用 `gcc-arm-8.3-2019.03-x86_64-aarch64-linux-gnu`。工具链路径可通过环境变量 `TOOLSCHAIN_ARMV7_PATH` / `TOOLSCHAIN_ARMV8_PATH` 覆盖。

> **License**：所有示例均以空的序列号和密钥初始化 SDK（`static const char* sn = ""; static const char* key = "";`）。运行前请替换为你自己的 License，否则 `Library::Initialize` 会失败。

## 构建与运行

### Windows（Visual Studio）

1. 打开 `simple_demo_vs2022.sln`（或单个示例的 `.vcxproj`）。
2. 选择配置（`Debug`/`Release`、`Win32`/`x64`）并构建。
3. 可执行文件输出到 `bin/`，带配置后缀，例如 `bin/annotation_rel_x64_vs2022.exe`。

### Linux / macOS（CMake + RunDemo.sh）

`RunDemo.sh` 会构建并立即运行一个或多个示例：

```bash
./RunDemo.sh annotation                 # 构建并运行单个示例
./RunDemo.sh annotation pdf2text        # 构建并运行多个示例
./RunDemo.sh all                        # 构建并运行全部示例
./RunDemo.sh annotation dbg             # Debug 构建
```

生成的二进制文件命名为 `<sample>_<os><arch>`，例如 `annotation_linux64`、`pdf2text_mac64`、`ocr_linuxarmv8`。

### 手动 CMake 构建

```bash
cmake . -DPRJ_NAME=pdf2text
make
./pdf2text_linux64 --help
```

### ARM 交叉编译

```bash
cmake . -DPRJ_NAME=pdf2text -DCMAKE_TOOLCHAIN_FILE=./toolchain-armv7.cmake
make
```

## 命令行约定

大多数示例遵循统一约定：

- `-i, --input <path>` —— 输入文件（PDF、图片、DWG、文本等）。
- `-o, --output <path>` —— 输出文件或目录。
- `--action <type>` / `--op <type>` —— 要执行的操作。
- `--help` —— 打印用法并退出。

少数较旧的示例使用位置参数（见下表）。请始终以 `<sample> --help` 输出的选项列表为准。

## 示例速查表

### 查看与渲染

| 示例 | 命令 |
| --- | --- |
| `pdf2image` | `pdf2image --input <in.pdf> --output-dir <dir> [--pages all\|1,3-5] [--formats bmp,jpg,png,...]` |
| `pdfreflow` | `pdfreflow --input <in.pdf> --output <dir> [--mode single\|continuous\|both]` |
| `output_preview` | `output_preview --input <in.pdf> --output <out.bmp> --icc <icc profile>` |
| `async_load` | `async_load_xxx -i <in.pdf> -o <out.bmp>` |
| `pdfprint` | `pdfprint --input <in.pdf> [--printer <name> \| --output <dir>] [--start-page n] [--end-page n]` |
| `pdfwrapper` | `pdfwrapper --action <wrap\|payload\|info\|extract> --input <in.pdf> --output <out>` |

### 转换

| 示例 | 命令 |
| --- | --- |
| `pdf2text` | `pdf2text --input <in.pdf> --output <out.txt>` |
| `pdf2xml` | `pdf2xml --input <in.pdf> --output <out.xml> [--image-folder <dir>]` |
| `pdf2office` | `pdf2office --input <in.pdf> --output <out.docx> [--format word\|excel\|ppt]` |
| `office2pdf` | `office2pdf_xxx --input <office file> --output <out.pdf> --type <word\|excel\|ppt>` |
| `html2pdf` | `html2pdf_xxx -html <url or html path> -o <out.pdf>` |
| `image2pdf` | `image2pdf_xxx -o <out.pdf> -p <page>:<x>:<y>:<image> [-p ...]` |
| `text2pdf` | `text2pdf --input <in.txt> --output <out.pdf> [--font-name <name>] [--text-size n]` |
| `dwg2pdf` | `dwg2pdf_xxx -i <in.dwg> -o <out.pdf> --engine <path>` |

### 编辑与页面操作

| 示例 | 命令 |
| --- | --- |
| `annotation` | `annotation_xxx [--op add\|delete\|edit] [-i <in.pdf>] [-o <out.pdf>] [-p <page>] [--type <type>]` |
| `annotation_summary` | `annotation_summary -i <in.pdf> -o <out.pdf> [--layout 0-4]` |
| `bookmark` | `demo_bookmark --action <get\|add> -i <in.pdf> [options]` |
| `page_organization` | `page_organization --input-dir <dir> --output <out.pdf> --ops <insert,remove,import,move>` |
| `page_labels` | `page_labels --op <create\|delete\|update\|query> --input <in.pdf> [options]` |
| `pdflayer` | `pdflayer --action <query\|add\|modify\|remove\|move> --input <in.pdf> --output <out.pdf>` |
| `pdfheaderfooter` | `pdfheaderfooter --action <add\|update\|remove> --input <in.pdf> --output <out.pdf>` |
| `watermark` | `demo_watermark --action <add\|remove> --input <in.pdf> --output <out.pdf>` |
| `graphics_objects` | `graphics_objects -i <in.pdf> -o <out.pdf> --op <add\|remove> --object-types <text\|image\|path>` |
| `matrix` | `matrix <in.pdf> <out.pdf> --op <translate\|rotate\|scale\|shear> [params]` |
| `paragraph_editing` | `paragraph_editing --input <in.pdf> --output-dir <dir> [options]` |
| `marked_content` | `marked_content <in.pdf> <out.pdf>` |
| `pdfredact` | `pdfredact --input <in.pdf> --output <out.pdf> --text <pattern>` |
| `searchreplace` | `searchreplace --input <in.pdf> --output <out.pdf> --pattern <text> --replacement <text>` |
| `pdfflatten` | `pdfflatten --input <in.pdf> --output <out.pdf> [--option all\|no-annot\|no-form]` |

### 表单

| 示例 | 命令 |
| --- | --- |
| `form` | `demo_form -i <in.pdf> -o <out.pdf> --field-types <pushbutton,radiobutton,checkbox,textfield,listbox,combobox>` |
| `form_combination` | `demo_form_combination -i <in1> [in2 ...] -o <out.csv>` |
| `form_recognition` | `demo_form_recognition -i <in.pdf> -o <out.pdf>` |
| `fillsign` | `demo_fillsign -i <in.pdf> -o <out.pdf> --object-types <crossmark,checkmark,roundrect,line,dot,text,signature>` |
| `fdf` | `fdf -i <in.pdf> -o <out> --mode <pdf2fdf\|pdf2xfdf\|fdf2pdf\|xfdf2pdf> --type <annot\|form\|annot,form>` |
| `xfa_form` | `xfa_form --input <in.pdf> --output <dir> --action <export\|import\|reset>` |
| `electronictable` | `demo_electronictable --output <out.pdf> --data <data.json>` |

### 安全与签名

| 示例 | 命令 |
| --- | --- |
| `security` | `demo_security --action <std\|certificate\|drm\|custom\|rms\|all> --input <in.pdf> --output <dir>` |
| `signature` | `demo_signature --action <sign\|verify> --input <in.pdf> --output <dir> --cert-path <pfx>` |
| `pades` | `pades --input <in.pdf> --output <out.pdf> --cert <cert.pfx> --cert-password <pwd>` |
| `ltv` | `ltv -i <in.pdf> -o <out.pdf> --cert <cert> --cert-password <pwd> [--digest sha256]` |
| `paging_seal_signature` | `paging_seal_signature --input <in.pdf> --output <out.pdf> [options]` |

### 搜索

| 示例 | 命令 |
| --- | --- |
| `search` | `search --input <in.pdf> --output <out.txt> --pattern <text> [--match-case]` |
| `fulltextsearch` | `demo_fulltextsearch -i <in folder> -o <out folder> [options]` |

### OCR 与识别

| 示例 | 命令 |
| --- | --- |
| `ocr` | `ocr -type <0\|1\|2> -input <in.pdf> -output <out> -engine <ocr engine path> [-lang <langs>]` |
| `layout_recognition` | `layout_recognition <in.pdf> <out info path>` |
| `barcode` | `demo_barcode --type <code39\|code128\|ean8\|upca\|ean13\|itf\|pdf417\|qrcode> -o <out image>` |
| `bitmap_transform` | `demo_bitmap_transform --op <operation> -i <in bitmap> -o <out bitmap>` |

### 文档处理

| 示例 | 命令 |
| --- | --- |
| `pdfcombination` | `pdfcombination --input <a.pdf> [--bookmark <title>] ... --output <out.pdf>` |
| `pdfcompare` | `pdfcompare --base <base.pdf> --compared <cmp.pdf> --output <dir>` |
| `compliance` | `compliance -i <in.pdf> -o <out.pdf> --type <PDFA\|PDFX\|PDFE> --resource-dir <path>` |
| `preflight` | `preflight --action <list\|analyze\|fixup> [--input <in.pdf>] [--key <preflight key>]` |
| `optimization` | `optimization --input <in.pdf> --output <out.pdf> --options <bitmask>` |
| `portfolio` | `portfolio --action <create\|info> --input <path> --output <path>` |
| `attachment` | `attachment_xxx --action <get\|add\|delete\|edit> -i <in.pdf> -o <out> [--key <name>] [--attach <file>]` |
| `associated_files` | `associated_files_xxx -i <in.pdf> -o <out> --action <get\|add\|delete\|edit> --command <catalog\|page\|annot\|image\|form>` |
| `docinfo` | `docinfo -i <in.pdf> -o <out.txt> [--mode metadata\|view\|both]` |
| `psi` | `psi --output <dir> --points-file <points.txt> [--diameter n] [--color 0xRRGGBB]` |
| `taggedpdf` | `taggedpdf --input <in.pdf> --output <out.pdf> [--callback]` |

## 说明

- 需要额外运行时资源的示例（OCR 引擎、DWG 引擎、合规资源目录、ICC 配置文件、证书等）通过命令行选项传入对应路径，详见各示例的 `--help`。
- 在 macOS 上，`RunDemo.sh` 会为 `dwg2pdf` 示例从 `DWG_ENGINE_PATH` 导出 `LD_LIBRARY_PATH`。
- `signature`、`security` 和 `paging_seal_signature` 示例会额外链接 `libssl.a` / `libcrypto.a`。
