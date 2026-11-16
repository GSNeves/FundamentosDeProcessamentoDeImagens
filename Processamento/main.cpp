#include "Processamento.h"
#include <QtWidgets/QApplication>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc.hpp>

using namespace cv;
using namespace std;

Mat imagem; //imagem original
Mat nova_imagem; //imagem nova
Mat imagem_match; //imagem para match
bool aberto = false; //Impede de realizar operação caso não tenha carregado nenhuma imagem
bool copiado = false; //Impede de realizar operação caso não tenha copiado nenhuma imagem
bool copiadomatch = false; //Impede de realizar matching caso não tenha copiado nenhuma imagem
const double gaussiano[9] = {0.0625, 0.125, 0.0625, 0.125, 0.25, 0.125, 0.0625, 0.125, 0.0625};
const double laplaciano[9] = {0, -1, 0, -1, 4, -1, 0, -1, 0};
const double altasgenerico[9] = {-1, -1, -1, -1, 8, -1, -1, -1, -1};
double prewittx[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1};
double prewitty[9] = {-1, -1, -1, 0, 0, 0, 1, 1, 1};
double sobelx[9] = {-1, 0, 1, -2, 0, 2, -1, 0, 1};
double sobely[9] = {-1, -2, -1, 0, 0, 0, 1, 2, 1};

void deixa_cinza(Mat entrada) {
    //Aplica uma equação de luminância em todos os pixels
    int gray = 0;
    for (int i = 0; i < entrada.cols; i++) {
        for (int j = 0; j < entrada.rows; j++) {
            unsigned char* p = entrada.ptr(j, i); // Y first, X after

            gray = 0.299 * p[2] + 0.587 * p[1] + 0.114 * p[0];
            p[0] = gray;
            p[1] = gray;
            p[2] = gray;
        }
    }
}

bool ecolorida(Mat imagem) {
    //Retorna true se a imagem é colorida, e false caso contrário
    for (int i = 0; i < nova_imagem.cols; i++) {
        for (int j = 0; j < nova_imagem.rows; j++) {
            unsigned char* p = nova_imagem.ptr(j, i); // Y first, X after

            if (p[0] != p[1] || p[1] != p[2])
                return true;
        }
    }
    return false;
}

void tonaliza(Mat entrada, int tons) {
    //Faz com que tenha apenas "tons" tons em uma imagem de tons de cinza
    if (tons <= 0 || tons >= 256) {
        return;
    }
    int maior = -1;
    int menor = 300;

    //Procura a maior e menor luminância
    for (int i = 0; i < entrada.cols; i++) {
        for (int j = 0; j < entrada.rows; j++) {
            unsigned char* p = entrada.ptr(j, i); // Y first, X after

            if (p[0] > maior)
                maior = p[0];
            if (p[0] < menor)
                menor = p[0];
        }
    }
    int tam_int = (maior - menor + 1);
    if (tons >= tam_int)
        return;
    //Calcula o tamanho de cada "divisão" de pixels
    int tb = tam_int / tons;
    int cont = 0;
    //Verifica em que divisão cada pixel está e faz média entre limite inferior e superior da divisão para determinar luminância do pixel
    for (int i = 0; i < entrada.cols; i++) {
        for (int j = 0; j < entrada.rows; j++) {
            unsigned char* p = entrada.ptr(j, i); // Y first, X after
            cont = 1;
            while (p[0] >= menor + cont * tb) {
                cont++;
            }
                
            p[0] = ((menor + (cont-1) * tb) + (menor + cont * tb)) / 2;
            p[1] = p[0];
            p[2] = p[0];
        }
    }
}

void flip_h(Mat entrada) {
    //Espelha horizontalmente a imagem

    int novo[3] = { 0 }; //Variavel auxiliar para que não se perca pixels na troca
    for (int i = 0; i < entrada.cols / 2; i++) {
        for (int j = 0; j < entrada.rows; j++) {
            unsigned char* p = entrada.ptr(j, i); // Y first, X after
            unsigned char* q = entrada.ptr(j, entrada.cols - i - 1);



            novo[0] = p[0];
            novo[1] = p[1];
            novo[2] = p[2];

            p[0] = q[0];
            p[1] = q[1];
            p[2] = q[2];

            q[0] = novo[0];
            q[1] = novo[1];
            q[2] = novo[2];
        }
    }
}

void flip_v(Mat entrada) {
    //Espelha verticalmente a imagem

    int novo[3] = { 0 }; //Variavel auxiliar para que não se perca pixels na troca
    for (int i = 0; i < entrada.cols; i++) {
        for (int j = 0; j < entrada.rows / 2; j++) {
            unsigned char* p = entrada.ptr(j, i); // Y first, X after
            unsigned char* q = entrada.ptr(entrada.rows - j - 1, i);



            novo[0] = p[0];
            novo[1] = p[1];
            novo[2] = p[2];

            p[0] = q[0];
            p[1] = q[1];
            p[2] = q[2];

            q[0] = novo[0];
            q[1] = novo[1];
            q[2] = novo[2];
        }
    }
}

void calculahistograma(int hist[256], Mat entrada) {
    //Calcula o histograma de uma imagem
    if (copiado) {
        for (int i = 0; i < entrada.cols; i++) {
            for (int j = 0; j < entrada.rows; j++) {
                unsigned char* p = entrada.ptr(j, i); // Y first, X after

                hist[p[0]]++;


            }
        }
    }
}

bool equalarr(double arr1[9], double arr2[9]) {
    //Verifica se 2 arranjos DOUBLE de tamanho 9 são iguais
    for (int i = 0; i < 9; i++) {
        if (arr1[i] != arr2[i]) {
            return false;
        }
    }
    return true;
}

void exibir_outro_histograma() {
    //É a função exibir_histograma novamente pois não consegui passar o nome da aba como parametro para poder abrir outra e mostrar 2 histogramas simultaneamente
    //Serve somente para exibir outro histograma simultaneamente 

    //Apenas realiza a operação se há imagem
    if (copiado) {
        //Transforma imagem em cinza se necessário
        if (ecolorida(nova_imagem)) {
            deixa_cinza(nova_imagem);
            imshow("Imagem asd", nova_imagem);
        }
        int histograma[256] = { 0 };
        float alpha = 0;
        int maior = 0;
        int maior_indice = 0;
        int escalar = 0;
        Mat hist(200, 256, CV_8UC3, Scalar(255, 255, 255));

        //Faz o histograma
        calculahistograma(histograma, nova_imagem);

        alpha = 200.0 / (nova_imagem.rows * nova_imagem.cols);

        maior = histograma[0];
        maior_indice = 0;
        //Acha o maior valor no histograma para definir o tamanho das barras verticais
        for (int i = 1; i <= 255; i++) {
            if (histograma[i] > maior) {
                maior = histograma[i];
                maior_indice = i;
            }
        }
        escalar = ceil(maior / 200);

        //Impede de ultrapassar o limite superior da imagem
        for (int i = 0; i <= 255; i++) {
            histograma[i] = histograma[i] / escalar;
            if (histograma[i] >= 200) {
                histograma[i] = 199;
            }
        }

        //Faz imagem do histograma
        for (int i = 0; i < hist.cols; i++) {
            for (int j = 0; j < histograma[i]; j++) {
                unsigned char* p = hist.ptr(hist.rows - j - 1, i); // Y first, X after
                p[0] = 65;
                p[1] = p[0];
                p[2] = p[0];

            }
        }

        //Mostra o histograma
        namedWindow("Histograma Final", WINDOW_AUTOSIZE);
        imshow("Histograma Final", hist);
        waitKey(0);

    }
}

void Processamento::exit() {
    destroyAllWindows();
    //Sai da aplicação
    QApplication::exit();
}

void Processamento::abreimagem() {
    //Abre a imagem com o endereço dado na caixa do widget
    QString a = ui.endereco->toPlainText();
    imagem = imread(a.toStdString());
    if (imagem.empty()) {
        //Impede que o programa crashe quando o endereço é invalido, nesse caso nada acontece
        return;
    }
    //Abre as janelas que irão mostrar as imagens
    namedWindow("Imagem_Original", WINDOW_AUTOSIZE);
    namedWindow("Imagem_Nova", WINDOW_AUTOSIZE);
    imshow("Imagem_Original", imagem);
    waitKey(0);
    //Diz que foi carregada uma imagem
    aberto = true;
}

void Processamento::abrematch() {
    //Abre a imagem com o endereço dado na caixa do widget
    QString a = ui.endereco_2->toPlainText();
    imagem_match = imread(a.toStdString());
    if (imagem_match.empty()) {
        //Impede que o programa crashe quando o endereço é invalido, nesse caso nada acontece
        return;
    }
    //Abre as janelas que irão mostrar as imagens
    deixa_cinza(imagem_match);
    namedWindow("Imagem Match", WINDOW_AUTOSIZE);
    imshow("Imagem Match", imagem_match);
    waitKey(0);
    //Diz que foi carregada uma imagem
    copiadomatch = true;
}

void Processamento::copiaimagem() {
    //Copia a imagem original para uma nova
    if (aberto) {
        nova_imagem = imagem.clone();
        imshow("Imagem_Nova", nova_imagem);
        waitKey(0);
        //Diz que foi copiada uma imagem
        copiado = true;
    }
}

void Processamento::tornacinza() {
    //Aplica o deixa_cinza() na imagem
    if (copiado) {
        deixa_cinza(nova_imagem);
        imshow("Imagem_Nova", nova_imagem);
        waitKey(0);
    }

}

void Processamento::horizontal() {
    //Aplica o flip_h() na imagem
    if (aberto) {
        flip_h(imagem);
        imshow("Imagem_Original", imagem);
        waitKey(0);
    }
}

void Processamento::vertical() {
    //Aplica o flip_v() na imagem
    if (aberto) {
        flip_v(imagem);
        imshow("Imagem_Original", imagem);
        waitKey(0);
    }
}

void Processamento::tone() {
    //Aplica o tonaliza() na imagem
    if (copiado) {
        tonaliza(nova_imagem, ui.tons->value());
        imshow("Imagem_Nova", nova_imagem);
        waitKey(0);
    }
}

void Processamento::save() {
    //Salva imagem nova como "saida.jpg"
    if (copiado) {
        imwrite("saida.jpg", nova_imagem);
    }
}

void Processamento::alterarbrilho() {
    //Impede de realizar a operação se não há imagem
    if (copiado) {
        //Percorre a imagem
        for (int i = 0; i < nova_imagem.cols; i++) {
            for (int j = 0; j < nova_imagem.rows; j++) {
                int aux[3];
                unsigned char* p = nova_imagem.ptr(j, i); // Y first, X after


                //Soma os 3 canais do pixel pelo valor dado
                for (int k = 0; k <= 2; k++) {
                    aux[k] = p[k];
                    aux[k] += ui.qntbrilho->value();

                    //Impede de exceder os valores
                    if (aux[k] > 255) {
                        aux[k] = 255;
                    }
                    else if (p[k] < 0) {
                        aux[k] = 0;
                    }
                    p[k] = aux[k];
                }


            }
        }
        //Mostra a imagem com novo brilho
        imshow("Imagem_Nova", nova_imagem);
        waitKey(0);
    }
}

void Processamento::alterarcontraste() {
    //Impede de realizar a operação se não há imagem
    if (copiado) {
        //Percorre a imagem
        for (int i = 0; i < nova_imagem.cols; i++) {
            for (int j = 0; j < nova_imagem.rows; j++) {
                double aux[3];
                unsigned char* p = nova_imagem.ptr(j, i); // Y first, X after

                //Multiplica os 3 canais do pixel pelo valor dado
                for (int k = 0; k <= 2; k++) {
                    aux[k] = p[k] * ui.qntcontraste->value();

                    //Impede de exceder os valores
                    if (aux[k] > 255) {
                        aux[k] = 255;
                    }

                    p[k] = aux[k];
                }


            }
        }
        //Mostra a imagem com novo contraste
        imshow("Imagem_Nova", nova_imagem);
        waitKey(0);
    }
}

void Processamento::negativar() {
    //Impede de realizar a operação se não há imagem
    if (copiado) {
        //Percorre a imagem
        for (int i = 0; i < nova_imagem.cols; i++) {
            for (int j = 0; j < nova_imagem.rows; j++) {
                unsigned char* p = nova_imagem.ptr(j, i); // Y first, X after

                //Subtrai de 255 o valor do canal k do pixel p
                for (int k = 0; k <= 2; k++) {
                    p[k] = 255 - p[k];
                }

            }
        }
        //Mostra a imagem negativada
        imshow("Imagem_Nova", nova_imagem);
        waitKey(0);
    }
}

void Processamento::zoomout() {
    //Tira zoom da imagem, diminuindo-a, fazendo a média dos valores dos pixels ques estão dentro de um retângulo que percorre a imagem

    //Impede de realizar a operação se não há imagem
    if (copiado) {
        int sx = ui.hor->value();
        int sy = ui.ver->value();
        int pixels;
        Mat grHistogram(ceil(nova_imagem.rows / sy), ceil(nova_imagem.cols / sx), CV_8UC3, Scalar(0, 0, 0));
        int new_i = 0;
        int new_j = 0;

        //Percorre a imagem
        for (int i = 0; i < nova_imagem.cols - sx; i = i + sx) {
            for (int j = 0; j < nova_imagem.rows - sy; j = j + sy) {
                int pixels = sx * sy;
                int x;
                int y;
                int soma[3] = { 0 };
                bool invalido = false;

                //Verifica os pixels dentro do retangulo que percorre a imagem
                for (x = 0; x < sx; x++) {
                    for (y = 0; y < sy; y++) {
                        //Se o pixel está fora da imagem, remove ele do cálculo da média
                        if (i + x >= nova_imagem.cols) {
                            pixels--;
                        }
                        else if (j + y >= nova_imagem.rows) {
                            pixels--;
                        }
                        else {
                            unsigned char* p = nova_imagem.ptr(j + y, i + x); // Y first, X after
                            for (int k = 0; k <= 2; k++) {
                                soma[k] += p[k];
                            }
                        }


                    }
                }
                unsigned char* q = grHistogram.ptr(j / sy, i / sx); // Y first, X after

                //Faz a média dos pixels
                if (pixels != 0) {
                    soma[0] = soma[0] / pixels;
                    q[0] = soma[0];
                    soma[1] = soma[1] / pixels;
                    q[1] = soma[1];
                    soma[2] = soma[2] / pixels;
                    q[2] = soma[2];
                }


                new_j++;
            }
            new_i++;
        }
        //Mostra a imagem nova
        nova_imagem = grHistogram.clone();
        imshow("Imagem_Nova", nova_imagem);
        waitKey(0);
    }

}

void Processamento::zoomin() {
    //Tira zoom da imagem, aumentando-a

    //Impede de realizar a operação se não há imagem
    if (copiado) {
        Mat grHistogram(nova_imagem.rows * 2 - 1, nova_imagem.cols * 2 - 1, CV_8UC3, Scalar(0, 0, 0));

        //Passa os pixels de 'p' para 'q'
        for (int i = 0; i < nova_imagem.cols; i++) {
            for (int j = 0; j < nova_imagem.rows; j++) {
                unsigned char* p = nova_imagem.ptr(j, i); // Y first, X after
                unsigned char* q = grHistogram.ptr(2 * j, 2 * i); // Y first, X after

                for (int k = 0; k <= 2; k++) {
                    q[k] = p[k];
                }

            }
        }

        //Faz a média entre as colunas de 'q'
        for (int i = 0; i < grHistogram.cols; i++) {
            for (int j = 1; j < grHistogram.rows - 2; j = j + 2) {

                unsigned char* ant = grHistogram.ptr(j - 1, i); // Y first, X after
                unsigned char* q = grHistogram.ptr(j, i); // Y first, X after
                unsigned char* prox = grHistogram.ptr(j + 1, i); // Y first, X after

                for (int k = 0; k <= 2; k++) {
                    q[k] = (ant[k] + prox[k]) / 2;
                }

            }
        }

        //Faz a média entre as linhas de 'q'
        for (int j = 0; j < grHistogram.rows; j++) {
            for (int i = 1; i < grHistogram.cols - 2; i = i + 2) {

                unsigned char* ant = grHistogram.ptr(j, i - 1); // Y first, X after
                unsigned char* q = grHistogram.ptr(j, i); // Y first, X after
                unsigned char* prox = grHistogram.ptr(j, i + 1); // Y first, X after

                for (int k = 0; k <= 2; k++) {
                    q[k] = (ant[k] + prox[k]) / 2;
                }

            }
        }
        //mostra a imagem
        nova_imagem = grHistogram.clone();
        imshow("Imagem_Nova", nova_imagem);
        waitKey(0);


    }
}

void Processamento::gira() {
    //Rotaciona a imagem em 90º, com a orientação definida pelo usuário

    //Impede de realizar a operação se não há imagem
    if (copiado) {
        Mat grHistogram(nova_imagem.cols, nova_imagem.rows, CV_8UC3, Scalar(0, 0, 0));

        //Gira em sentido horário
        if (ui.gira->checkState() == Qt::Checked) {

            for (int j = 0; j < nova_imagem.rows; j++) {
                for (int i = 0; i < nova_imagem.cols; i++) {
                    unsigned char* p = nova_imagem.ptr(j, i); // Y first, X after
                    unsigned char* q = grHistogram.ptr(i, grHistogram.cols - j - 1); // Y first, X after

                    for (int k = 0; k <= 2; k++) {
                        q[k] = p[k];
                    }

                }
            }
        }
        //Gira em sentido anti-horário
        else {
            for (int j = 0; j < nova_imagem.rows; j++) {
                for (int i = 0; i < nova_imagem.cols; i++) {
                    unsigned char* p = nova_imagem.ptr(j, i); // Y first, X after
                    unsigned char* q = grHistogram.ptr(grHistogram.rows - i - 1, j); // Y first, X after

                    for (int k = 0; k <= 2; k++) {
                        q[k] = p[k];
                    }

                }
            }
        }
        //Mostra a imagem rotacionada
        nova_imagem = grHistogram.clone();
        imshow("Imagem_Nova", nova_imagem);
        waitKey(0);

    }
}

void Processamento::convolucao() {
    //Aplica convolução na nova imagem

    //Impede de realizar a operação se não há imagem e se ela não é em tom de cinza
    if (copiado && !(ecolorida(nova_imagem))) {
        double matriz[9] = {ui.c0->value(), ui.c1->value(), ui.c2->value(), ui.c3->value(), ui.c4->value(), ui.c5->value(), ui.c6->value(), ui.c7->value(), ui.c8->value()};
        Mat grHistogram(nova_imagem.rows-2, nova_imagem.cols-2, CV_8UC3, Scalar(0, 0, 0));
        bool soma127 = false;
        //Se a matriz dada for prewitt ou sobel, soma 127 aos pixels
        if (equalarr(matriz, prewittx) || equalarr(matriz, prewitty) || equalarr(matriz, sobelx) || equalarr(matriz, sobely)) {
            soma127 = true;
        }

        //Percorre a imagem
        for (int i = 1; i < nova_imagem.cols-1; i++) {
            for (int j = 1; j < nova_imagem.rows-1; j++) {
                unsigned char* p[9];
                unsigned char* r = grHistogram.ptr(j-1, i-1);
                double q[9] = { 0 };
                int k = 0;

                for (int a = -1; a <= 1; a++) {
                    for (int b = -1; b <= 1; b++) {
                        p[k] = nova_imagem.ptr(j + a, i + b);
                        q[k] = p[k][0];
                        k++;
                    }
                }
                //Multiplica os pixels com os valores da matriz de convolução
                q[4] = q[0] * matriz[8] + q[1] * matriz[7] + q[2] * matriz[6] + q[3] * matriz[5] + q[4] * matriz[4] + q[5] * matriz[3] + q[6] * matriz[2] + q[7] * matriz[1] + q[8] * matriz[0];

                if (soma127) {
                    q[4] += 127;
                }
                    
                //Impede que exceda os valores dos pixels
                if (q[4] > 255) {
                    q[4] = 255;
                }
                else if (q[4] < 0) {
                    q[4] = 0;
                }
                r[0] = q[4];
                r[1] = q[4];
                r[2] = q[4];

            }
        }
        //Mostra a imagem com o filtro
        nova_imagem = grHistogram.clone();
        imshow("Imagem_Nova", nova_imagem);
        waitKey(0);
    }
}

void Processamento::equalizacao() {
    //Faz a operação de equalização na nova_imagem

    //Mostra histograma se é cinza
    if (!(ecolorida(nova_imagem))) {
        exibir_histograma();
    }
    //Impede de realizar a operação se não há imagem
    if (copiado) {
        int histogram[256] = { 0 }, hist_cum[256] = { 0 };
        Mat cinza(nova_imagem.cols, nova_imagem.rows, CV_8UC3, Scalar(0, 0, 0));
        float alpha = 255.0 / (nova_imagem.rows * nova_imagem.cols);
        cinza = nova_imagem.clone();
        deixa_cinza(cinza);

        //Faz o histograma
        calculahistograma(histogram, cinza);

        hist_cum[0] = round(alpha * histogram[0]);

        //Faz histograma cumulativo
        for (int i = 0; i <= 255; i++) {
            hist_cum[i] = hist_cum[i - 1] + round(alpha * histogram[i]);
        }

        //Equaliza a imagem
        for (int j = 0; j < nova_imagem.rows; j++) {
            for (int i = 0; i < nova_imagem.cols; i++) {
                unsigned char* p = nova_imagem.ptr(j, i); // Y first, X after
                for (int c = 0; c <= 2; c++) {
                    p[c] = hist_cum[p[c]];

                }
            }
        }
        //Mostra histograma se é cinza
        if (!(ecolorida(nova_imagem))) {
            exibir_outro_histograma();
        }
        //Mostra a imagem equalizada
        imshow("Imagem_Nova", nova_imagem);
        waitKey(0);

    }
}

int find_target_shade_level_closest_to(int hist_src_cum[256], int hist_target_cum[256], int shade) {
    //Compara 2 arranjos e verifica qual valor do segundo arranjo está mais perto do valor de hist_src_cum[shade]
    int menor = hist_src_cum[shade] - hist_target_cum[0];
    int menor_indice = 0;
    int diff = 0;
    //Acha a menor diferença
    for (int i = 1; i <= 255; i++) {
        diff = hist_src_cum[shade] - hist_target_cum[i];
        if (abs(diff) < abs(menor)) {
            menor = diff;
            menor_indice = i;
        }
    }
    //retorna o indice
    return menor_indice;
}

void Processamento::histogram_matching() {
    //Faz histogram matching entre 2 imagens cinzas

    //Apenas realiza a operação se há as 2 imagens
    if (copiado && copiadomatch) {
        //Apenas realiza a operação se as 2 são cinzas
        if (!(ecolorida(nova_imagem)) && !(ecolorida(imagem_match))) {
            int hist_src[256] = { 0 }, hist_target[256] = { 0 }, hist_src_cum[256] = { 0 }, hist_target_cum[256] = { 0 };
            int HM[256] = { 0 };
            float alpha;

            Mat src(nova_imagem.cols, nova_imagem.rows, CV_8UC3, Scalar(0, 0, 0));
            src = nova_imagem.clone();
            Mat dst(imagem_match.cols, imagem_match.rows, CV_8UC3, Scalar(0, 0, 0));
            dst = imagem_match.clone();

            //Faz histogramas das 2 imagens
            calculahistograma(hist_src, src);
            calculahistograma(hist_target, dst);

            //Faz histograma cumulativo da primeira imagem
            alpha = 255.0 / (src.rows * src.cols);
            hist_src_cum[0] = round(alpha * hist_src[0]);
            for (int i = 0; i <= 255; i++) {
                hist_src_cum[i] = hist_src_cum[i - 1] + round(alpha * hist_src[i]);
            }

            //Faz histograma cumulativo da segunda imagem
            alpha = 255.0 / (dst.rows * dst.cols);
            hist_target_cum[0] = round(alpha * hist_target[0]);
            for (int i = 0; i <= 255; i++) {
                hist_target_cum[i] = hist_target_cum[i - 1] + round(alpha * hist_target[i]);
            }

            //Faz vetor com o mapeamento
            for (int i = 0; i <= 255; i++) {
                HM[i] = find_target_shade_level_closest_to(hist_src_cum, hist_target_cum, i);
            }

            //Faz o matching
            for (int j = 0; j < nova_imagem.rows; j++) {
                for (int i = 0; i < nova_imagem.cols; i++) {
                    unsigned char* p = nova_imagem.ptr(j, i); // Y first, X after
                    p[0] = HM[p[0]];
                    p[1] = p[0];
                    p[2] = p[0];
                }
            }
            //Mostra nova imagem
            imshow("Imagem_Nova", nova_imagem);
            waitKey(0);
        }
    }

}


void Processamento::exibir_histograma() {
    //Faz um histograma de nova_imagem

    //Apenas realiza a operação se há imagem
    if (copiado) {
        //Transforma imagem em cinza se necessário
        if (ecolorida(nova_imagem)) {
            deixa_cinza(nova_imagem);
            imshow("Imagem_Nova", nova_imagem);
        }
        int histograma[256] = { 0 };
        float alpha = 0;
        int maior = 0;
        int maior_indice = 0;
        int escalar = 0;
        Mat hist(200, 256, CV_8UC3, Scalar(255, 255, 255));

        //Faz o histograma
        calculahistograma(histograma, nova_imagem);

        alpha = 200.0 / (nova_imagem.rows * nova_imagem.cols);

        maior = histograma[0];
        maior_indice = 0;
        //Acha o maior valor no histograma para definir o tamanho das barras verticais
        for (int i = 1; i <= 255; i++) {
            if (histograma[i] > maior) {
                maior = histograma[i];
                maior_indice = i;
            }
        }
        escalar = ceil(maior / 200);

        //Impede de ultrapassar o limite superior da imagem
        for (int i = 0; i <= 255; i++) {
            histograma[i] = histograma[i] / escalar;
            if (histograma[i] >= 200) {
                histograma[i] = 199;
            }
        }

        //Faz imagem do histograma
        for (int i = 0; i < hist.cols; i++) {
            for (int j = 0; j < histograma[i]; j++) {
                unsigned char* p = hist.ptr(hist.rows - j - 1, i); // Y first, X after
                p[0] = 65;
                p[1] = p[0];
                p[2] = p[0];

            }
        }

        //Mostra o histograma
        namedWindow("Histograma", WINDOW_AUTOSIZE);
        imshow("Histograma", hist);
        waitKey(0);
        
    }
}

Processamento::Processamento(QWidget* parent)
    //Configuração dos botões do widget
    : QMainWindow(parent)
{
    ui.setupUi(this);
    connect(ui.pushbutton, SIGNAL(clicked()), this, SLOT(exit()));
    connect(ui.abre, SIGNAL(clicked()), this, SLOT(abreimagem()));
    connect(ui.copia, SIGNAL(clicked()), this, SLOT(copiaimagem()));
    connect(ui.cinza, SIGNAL(clicked()), this, SLOT(tornacinza()));
    connect(ui.horiz, SIGNAL(clicked()), this, SLOT(horizontal()));
    connect(ui.vert, SIGNAL(clicked()), this, SLOT(vertical()));
    connect(ui.quantizar, SIGNAL(clicked()), this, SLOT(tone()));
    connect(ui.save, SIGNAL(clicked()), this, SLOT(save()));
    connect(ui.brilho, SIGNAL(clicked()), this, SLOT(alterarbrilho()));
    connect(ui.contraste, SIGNAL(clicked()), this, SLOT(alterarcontraste()));
    connect(ui.negativo, SIGNAL(clicked()), this, SLOT(negativar()));
    connect(ui.zoomout, SIGNAL(clicked()), this, SLOT(zoomout()));
    connect(ui.zoomin, SIGNAL(clicked()), this, SLOT(zoomin()));
    connect(ui.girabutton, SIGNAL(clicked()), this, SLOT(gira()));
    connect(ui.convolucao, SIGNAL(clicked()), this, SLOT(convolucao()));
    connect(ui.equalizar, SIGNAL(clicked()), this, SLOT(equalizacao()));
    connect(ui.abrematch, SIGNAL(clicked()), this, SLOT(abrematch()));
    connect(ui.histmatch, SIGNAL(clicked()), this, SLOT(histogram_matching()));
    connect(ui.hist, SIGNAL(clicked()), this, SLOT(exibir_histograma()));
}

int main(int argc, char *argv[])
//Função main
{
    //Abre o widget
    QApplication a(argc, argv);
    Processamento w;
    w.show();
    return a.exec();
}
