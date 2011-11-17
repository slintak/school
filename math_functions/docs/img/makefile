# vychozi obrazky
SRC=datum1.fig datum2.fig
# pozadovany format pro LaTeX
DEST=datum1.eps datum2.eps

.SUFFIXES: .fig .eps .png .jpg

all: $(DEST)

.fig.eps:
	fig2dev -L eps $< $@

.png.eps:
	convert $< $@

.jpg.eps:
	convert $< $@
