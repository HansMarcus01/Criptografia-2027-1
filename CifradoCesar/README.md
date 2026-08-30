# UNAM Engineering LaTeX Template

A modular, feature-rich LaTeX template specifically structured for academic assignments, reports, and projects at the Universidad Nacional Autónoma de México (UNAM), Facultad de Ingeniería.

## Project Structure

The repository is organized to separate the document content from the configuration files, keeping your main workspace clean:

*   **`doc.tex`**: The main document file where you will write your content. It uses the `article` class (12pt, letterpaper) and pulls in all the modular configurations.
*   **`img/`**: Directory containing the UNAM and FI logos in both color and black-and-white (PNG and SVG formats).
*   **`template/`**: Contains the modular configuration files that build the document:
    *   `preamble.tex`: Loads all required packages and global configurations.
    *   `listings_config.tex`: Contains predefined styles for source code syntax highlighting.
    *   `title_page.tex`: A custom title page layout featuring the university logos and assignment metadata.
    *   `header.tex`: Sets up the page headers using the `fancyhdr` package.
    *   `biblio.tex`: A template for custom bibliography entries using the `biblio` environment.

## Features

*   **Comprehensive Package Support:** The preamble is fully loaded for engineering and science papers, including packages for advanced math (`amsmath`, `nicematrix`), circuitry (`circuitikz`), plotting (`pgfplots`), and complex tables (`tabularx`, `multirow`).
*   **Ready-to-Use Code Blocks:** The `listings` package is pre-configured to handle Spanish characters (á, é, ñ, etc.) and includes custom syntax highlighting styles for **SQL**, **VHDL**, **C**, **C++**, **Python**, **Bash**, and **Assembly** (`[x86masm]`).
*   **Custom Title Page:** Includes absolutely positioned UNAM and FI logos with placeholders for the subject, professor, student details, and delivery date.
*   **Clickable Hyperlinks:** Pre-configured URL and cross-reference setups with automatic line breaks for long links.

## How to Use & Customize

Before compiling `doc.tex` for the first time, make sure to update the placeholder information in the template files:

1.  **Update the Title Page (`template/title_page.tex`):**
    Search for the placeholders enclosed in angle brackets and replace them with your assignment's data:
    *   `<titulo_trabajo>`
    *   `<facultad>`, `<materia>`, `<profesor>`, `<semestre>`, `<grupo>`
    *   `<nombre_estudiante>`, `<num_cuenta>`, `<fecha_entrega>`
2.  **Update the Header (`template/header.tex`):**
    Search for the placeholders enclosed in angle brackets and replace them with your assignment's data:
    *  `<nombre_estudiante>`, `<num_cuenta>`
    *   `<asignatura>`, `<grupo>`
3.  **Manage Bibliography (`template/biblio.tex`):**
    Add your references inside the `\<begin{biblio}` environment. The template includes an example referencing a blog post about database systems.
4.  **Write and Compile (`doc.tex`):**
    Add your document content below `\section{1st section}` and compile using your preferred LaTeX compiler (e.g., pdfLaTeX or LuaLaTeX).

### Compilation with `pdflatex`

```bash
pdflatex <main_file.tex>
pdflatex doc.tex
```

**Important: This template should not be modified directly it should be copied to the target directory for the project. Also, run `pdflatex` twice to ensure that all references and citations are correctly resolved.**