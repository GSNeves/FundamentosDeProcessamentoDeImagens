# Processamento de Imagens — FPI (Lab 2)

Aplicação desktop desenvolvida em **C++** com **OpenCV** e **Qt** para a disciplina de **Fundamentos de Processamento de Imagens (FPI)** da UFRGS.

## Funcionalidades

### Parte 1 — Operações Básicas, Ponto a Ponto e Histogramas
- **Conversão para Tons de Cinza (Luminância):** $0.299 R + 0.587 G + 0.114 B$
- **Espelhamento:** Horizontal e Vertical
- **Quantização:** Redução do número de tons em imagens em escala de cinza
- **Ajuste de Brilho e Contraste**
- **Negativo da Imagem**
- **Cálculo e Exibição de Histograma**
- **Equalização de Histograma** (com exibição do histograma antes e depois)
- **Histogram Matching** entre duas imagens em tons de cinza

### Parte 2 — Transformações Geométricas e Convolução Espacial
- **Zoom Out:** Redução com fatores $S_x \times S_y$ via média dos pixels por bloco
- **Zoom In:** Ampliação $2\times$ por interpolação bilinear (média dos vizinhos)
- **Rotação de 90°:** Sentido horário e anti-horário
- **Convolução 2D com Kernel $3\times 3$ Arbitrário:**
  - Suporte a filtros passa-baixas e passa-altas (Gaussiano, Laplaciano, Passa-Altas Genérico)
  - Detecção de bordas (Prewitt $H_x$/$H_y$ e Sobel $H_x$/$H_y$, com soma de offset 127)

## Estrutura do Projeto

- `Processamento/main.cpp` — Implementação das operações de processamento de imagens e slots da interface
- `Processamento/Processamento.h` — Declaração da classe principal da janela Qt
- `Processamento/Processamento.ui` — Layout da interface gráfica (Qt Designer)
- `Processamento/Processamento.sln` / `Processamento.vcxproj` — Solução e projeto do Visual Studio

## Requisitos

- **Visual Studio 2022** (ou compatível com toolset `v143`) com extensão **Qt VS Tools**
- **Qt 6** (configurado para `6.2.3_msvc2019_64` ou ajustável nas propriedades do projeto)
- **OpenCV 4** (ex.: `opencv_world455d.lib` em `C:\OpenCV\opencv\build`)
