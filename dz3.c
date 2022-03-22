#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <stdbool.h>
#include <ctype.h>

//структуры графа с матрицей смежности
typedef struct {
	uint64_t** adj_matrix; 
	uint64_t count_vertices;
	uint64_t count_edges;
} graph;

//подсчёт длины таблицы, количества рёбер 
uint64_t count_width(FILE* file) {
	rewind(file);
	uint64_t count = 0;
	char ch;
	do {
		if (isdigit(ch = fgetc(file))) count++;
	} while (ch != '\n');
	return count;
}

//под счёт высоты таблицы, количества вершин 
uint64_t count_height(FILE* file) {
	rewind(file);
	uint64_t count = 0;
	while (!feof(file)) {
        if (fgetc(file) == '\n') count++;
    }
	return count;
}

//считывает матрицу инцидетности из файла
void fread_inc_matrix(bool** inc_matrix, FILE* file, uint64_t count_edges, uint64_t count_vertices) {
	rewind(file);
	uint8_t buffer;
	for (uint64_t i = 0; i < count_vertices; i++) {
		for (uint64_t j = 0; j < count_edges; j++) {
			fscanf(file, "%" SCNu8 "", &buffer);
            if (buffer) inc_matrix[i][j] = true;
		}
	}
}

//переводит матрицу инцидетности в матрциу смежности и записывает в граф
void convert_inc_matrix_and_write_adj_graph(graph* cur_graph, bool** inc_matrix) {
	int i1, i2;
	for (uint64_t i = 0; i < cur_graph->count_edges; i++) {
		i1 = i2 = -1;
		for (uint64_t j = 0; j < cur_graph->count_vertices; j++) {
			if (inc_matrix[j][i]) {
				if (i1 == -1) i1 = i2 = j;
				else i2 = j;
			}
		}
		if (i1 == i2) cur_graph->adj_matrix[i1][i1]++;
		else {
			cur_graph->adj_matrix[i1][i2]++;
			cur_graph->adj_matrix[i2][i1]++;
		}
	}
}

//считывает граф по матрице инцидетности из файла
void fread_graph (graph* cur_graph, FILE* file) {
	//инициализируем двойной массив матрицы инцидетности
	bool** inc_matrix = (bool**) malloc(cur_graph->count_vertices * sizeof(bool*));
	for (uint64_t i = 0; i < cur_graph->count_vertices; i++) {
		inc_matrix[i] = (bool*) calloc(cur_graph->count_edges, sizeof(bool));
	}
	
	//считываем матрицу инцидетности из файла
	fread_inc_matrix(inc_matrix, file, cur_graph->count_edges, cur_graph->count_vertices);
	
	//переводим матрицу инцидетности в матрциу смежности и записываем в граф
	convert_inc_matrix_and_write_adj_graph(cur_graph, inc_matrix);
	
	//очищаем память от матрицы инцидетности
	for (uint64_t i = 0; i < cur_graph->count_vertices; i++) {
		free(inc_matrix[i]);
	}
	free(inc_matrix);
}

//инициализация графа, выделение на него памяти
graph* init_graph(FILE* file) {
	graph* new_graph = (graph*) malloc(sizeof(graph)); 
	new_graph->count_vertices = count_height(file);
	new_graph->count_edges = count_width(file);
	
	new_graph->adj_matrix = (uint64_t**) malloc(new_graph->count_vertices * sizeof(uint64_t*));
	for (uint64_t i = 0; i < new_graph->count_vertices; i++){
		new_graph->adj_matrix[i] = (uint64_t*) calloc(new_graph->count_vertices, sizeof(uint64_t));
	}
	
	fread_graph(new_graph, file);
	return new_graph;
}

//очищаем память от графа
void del_graph(graph* cur_graph) {
	for (uint64_t i = 0; i < cur_graph->count_vertices; i++) {
		free(cur_graph->adj_matrix[i]);
	}
	free(cur_graph->adj_matrix);
	free(cur_graph);
}

//проверка, что граф простой
bool check_simple_graph(graph* cur_graph) {
	for (uint64_t i = 0; i < cur_graph->count_vertices; i++) {
		if (cur_graph->adj_matrix[i][i] != 0) return false;
		for (uint64_t j = i + 1; j < cur_graph->count_vertices; j++) {
			if (cur_graph->adj_matrix[i][j] > 1) return false;
		}
	}
	return true;
}

//проверка, что граф связный
int check_connected_graph (graph* cur_graph) {
	if (check_simple_graph(cur_graph)) {
		if (cur_graph->count_edges > ((cur_graph->count_vertices - 1)*(cur_graph->count_vertices - 2)/2)) 
			return 1;
		else return 0;
	} else return -1;
}

int main(int argc, char *argv[]) {
	if (argc != 2) {
		puts("Error: wrong argunets!\n");
		return -101;
	}  
	
	FILE* fread = fopen(argv[1], "r");
	graph* cur_graph = init_graph(fread); 
	fclose(fread);
	
	switch (check_connected_graph(cur_graph)) {
		case 1:
			puts("Graph is connected");
			del_graph(cur_graph);
			return 1;
		case 0:
			puts("The theorem fails");
			del_graph(cur_graph);
			return 0;
		case -1:
			puts("Graph is not simple, the theorem fails");
			del_graph(cur_graph);
			return -1;
	}
}
