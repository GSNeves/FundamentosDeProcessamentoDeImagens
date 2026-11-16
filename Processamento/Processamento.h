#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_Processamento.h"

using namespace std;

class Processamento : public QMainWindow
{
    Q_OBJECT

public:
    Processamento(QWidget *parent = Q_NULLPTR);
private:
    Ui::ProcessamentoClass ui;
private slots:
    void exit();
    void abreimagem();
    void copiaimagem();
    void tornacinza();
    void horizontal();
    void vertical();
    void tone();
    void save();
    void alterarbrilho();
    void alterarcontraste();
    void negativar();
    void zoomout();
    void zoomin();
    void gira();
    void convolucao();
    void equalizacao();
    void histogram_matching();
    void abrematch();
    void exibir_histograma();
};
