#include "raylib/raylib.h"
#define RAYGUI_IMPLEMENTATION
#include "raylib/raygui.h"
#include "stdio.h"

#define LARGURA 480
#define ALTURA  620


#define LARGURA_PROGRESSO 200
#define ALTURA_PROGRESSO   30

#define X_PROGRESSO ((LARGURA / 2) - (LARGURA_PROGRESSO / 2))
#define Y_PROGRESSO ((ALTURA / 2) - ((ALTURA_PROGRESSO)))

#define LARGURA_BTN 150
#define ALTURA_BTN   60

#define X_BTN ((LARGURA / 2) - (LARGURA_BTN / 2))

#define Y_BTN ((Y_PROGRESSO) + ALTURA_BTN)

#define TEMPO_PARA_CONCLUSAO_NORMAL ((struct Tempo){.minutos = 25, .segundos = 0})
#define TEMPO_DESCANSO ((struct Tempo){.minutos = 5, .segundos = 0})
#define TEMPO_DESCANSO_GRANDE ((struct Tempo){.minutos = 15, .segundos = 0})
enum Estado {
	NAO_COMECOU,
	CONTANDO,
	PAUSADO,
};


struct Tempo {
	int minutos;
	int segundos;
};

const char *tempo_para_string(struct Tempo tempo, char buffer[], size_t tamanho) {
	snprintf(buffer, tamanho, "%02d:%02d", tempo.minutos, tempo.segundos);
	return buffer;
}

float tempo_para_segundos(struct Tempo tempo) {
	return (float)((tempo.minutos * 60) + tempo.segundos);
}

void adiciona_um_segundo(struct Tempo *tempo) {
	if(tempo->segundos == 59) {
		tempo->minutos++;
		tempo->segundos = 0;
	} else {
		tempo->segundos++;
	}
}

int tempo_igual(struct Tempo t1, struct Tempo t2) {
	return t1.minutos == t2.minutos && t1.segundos == t2.segundos;
}


static char buffer_tempo_atual[255];
static char buffer_tempo_para_conclusao[255];

struct Pomodoro {
	struct Tempo tempo_atual;
	struct Tempo tempo_para_conclusao;
	float progresso;
	enum Estado estado;
	int contador; // se == 7 grande descanso senao descanso pequeno
};


struct Pomodoro cria_pomodoro() {
	return (struct Pomodoro) {
		.tempo_atual = (struct Tempo){.minutos = 0, .segundos = 0},
		.tempo_para_conclusao = (struct Tempo) TEMPO_PARA_CONCLUSAO_NORMAL,
		.progresso = 0,
		.estado = NAO_COMECOU,
		.contador = 1,
	};

}

int main(void) {

	struct Pomodoro pomodoro = cria_pomodoro();
	//Inicializando o som
	InitAudioDevice();

	if(!IsAudioDeviceReady()) {
		fprintf(stderr, "Não foi possível inicializar o áudio corretamente!\n");
		exit(1);
	}

	Sound audio = LoadSound("./audio.wav");

	// Inicializando o vídeo
	InitWindow(LARGURA, ALTURA, "POMODORO");
	SetTargetFPS(60);


	
	if(!IsSoundReady(audio)) {
		fprintf(stderr, "Não foi possível carregar o arquivo de áudio corretamente!\n");
		exit(1);
	}

	const char *label_botao_principal = "começar";


	double tempo;

	while(!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground(RAYWHITE);

		GuiProgressBar(
				(Rectangle){X_PROGRESSO, Y_PROGRESSO, LARGURA_PROGRESSO, ALTURA_PROGRESSO}, 
				tempo_para_string(pomodoro.tempo_atual, buffer_tempo_atual, 255), 
				tempo_para_string(pomodoro.tempo_para_conclusao, buffer_tempo_para_conclusao, 255), 
				&pomodoro.progresso, 
				0.0f,
				tempo_para_segundos(pomodoro.tempo_para_conclusao)

		);

		if(GuiButton((Rectangle){X_BTN, Y_BTN, LARGURA_BTN, ALTURA_BTN}, label_botao_principal)) {
			switch(pomodoro.estado) {
				case NAO_COMECOU:
					label_botao_principal = "pausar";
					pomodoro.estado = CONTANDO;
					tempo = GetTime();
					break;
				case CONTANDO:
					label_botao_principal = "continuar";
					pomodoro.estado = PAUSADO;
					break;
				case PAUSADO:
					label_botao_principal = "pausar";
					pomodoro.estado = CONTANDO;
					tempo = GetTime();
					break;
			}
		}

		EndDrawing();

		//checando se pode adicionar
		if(tempo_igual(pomodoro.tempo_atual, pomodoro.tempo_para_conclusao)) {
			if(tempo_igual(pomodoro.tempo_para_conclusao, TEMPO_PARA_CONCLUSAO_NORMAL)) { // se verdadeiro deve ir para o descanso
				if(pomodoro.contador == 7) {
					pomodoro.tempo_para_conclusao = TEMPO_DESCANSO_GRANDE;
					pomodoro.contador = 1;
				} else {
					pomodoro.tempo_para_conclusao = TEMPO_DESCANSO;
				}
				pomodoro.tempo_atual = (struct Tempo){0,0};
			} else { // Saiu do descanso
				pomodoro.tempo_para_conclusao = TEMPO_PARA_CONCLUSAO_NORMAL;
				pomodoro.tempo_atual = (struct Tempo){0,0};
				pomodoro.estado = NAO_COMECOU;
				label_botao_principal = "começar";
			}

			pomodoro.contador++;
			PlaySound(audio);
		}
		
		if(pomodoro.estado == CONTANDO && GetTime() - tempo >= 1){
			adiciona_um_segundo(&pomodoro.tempo_atual);	
			tempo = GetTime();
		}

		pomodoro.progresso = tempo_para_segundos(pomodoro.tempo_atual);

	}

	CloseWindow();
	UnloadSound(audio);
	CloseAudioDevice();

	return 0;
}
