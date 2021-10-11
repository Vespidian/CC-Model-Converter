#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "shared.h"

// Per model variables
bool skip_file = false;
char *path;

// Mesh data and types
unsigned char *mesh_data;
unsigned int data_length = 0;

unsigned int num_verts;
Vertex *verts;

unsigned int num_triangles;
Triangle *tris;

void ReadFile(char *file){
	if(strcmp(file + strlen(file) - 5, ".modl") == 0){
	// char *file_tmp = malloc(strlen(file) + 3);
	// if(strcmp(file + strlen(file) - 4, ".obj") == 0){
	// 	strcpy(file_tmp, file);
	// 	strcpy(file_tmp + strlen(file_tmp) - 4, ".modl");
	// 	file_tmp[strlen(file_tmp)] = 0;
	// }
		// Open the file
		FILE *modl_file;
		modl_file = fopen(file, "rb");
		if(modl_file == NULL){
			printf("Error opening file!\n");
			skip_file = true;
			return;
		}
		
		// Find length of the source file
		fseek(modl_file, 0, SEEK_END);
		int file_size = ftell(modl_file);
		fseek(modl_file, 0, SEEK_SET);
		data_length = file_size;

		// Allocate space for data
		mesh_data = malloc(file_size + 2);

		// Read to data array
		size_t error_check = fread(mesh_data, 1, file_size, modl_file);
		if(error_check != file_size){
			printf("Error reading file!\n");
			skip_file = true;
			return;
		}

		// Close the file
		fclose(modl_file);
	}else{
		printf("Skipping non .modl file (This program can only convert .modl file into .obj files)\n");
		skip_file = true;
	}
}

void WriteFile(char *file){
	if(num_verts > 0){
		printf("Creating .obj..\n");
		// Get rid of the .modl file type
		file[strlen(file) - 5] = 0;
		if(strrchr(file, '/') != NULL){
			file = strrchr(file, '/') + 1;
		}

		char file_path[64];
		sprintf(file_path, "%s.obj", file);

		errno = 0;
		FILE *obj_file;
		obj_file = fopen(file_path, "w");
		if(obj_file == NULL){
			printf("error: Could not write file (not enough permissions)\n");
			return;
		}

		// Write obj header
		fprintf(obj_file, "# modl2obj v1.0 by Pimy\n# https://github.com/Vespidian\n");

		// Writing positions
		for(int vertices = 0; vertices < num_verts; vertices++){
			fprintf(obj_file, "v %f %f %f\n", verts[vertices].pos.x, verts[vertices].pos.y, verts[vertices].pos.z);
		}

		// Writing texture coordinates
		for(int vertices = 0; vertices < num_verts; vertices++){
			fprintf(obj_file, "vt %f %f\n", verts[vertices].uv.x, verts[vertices].uv.y);
		}

		// Writing triangle faces
		for(int triangles = 0; triangles < num_triangles; triangles++){
			fprintf(obj_file, "f %d/%d %d/%d %d/%d\n", tris[triangles][0] + 1, tris[triangles][0] + 1, tris[triangles][1] + 1, tris[triangles][1] + 1, tris[triangles][2] + 1, tris[triangles][2] + 1);
		}

		fclose(obj_file);
	}else{
		printf("error: .modl file has no data! (0 vertices)\n");
	}
}

void ParseData(){
	if(data_length > 0){
		unsigned char *data_pointer = mesh_data;
		TypeConverter_tu value;
		printf("Reading bounding box..\n");
		printf("Bounds:\n");

		for(int i = 0; i < 6; i++){
			value.c1 = data_pointer[0];
			value.c2 = data_pointer[1];
			value.c3 = data_pointer[2];
			value.c4 = data_pointer[3];
			data_pointer += 4;
		}
		// printf("y: %f - %f\n", bounds[0].y, bounds[1].y);
		// printf("z: %f - %f\n", bounds[0].z, bounds[1].z);

		data_pointer += 1; // Delimiting zero

		value.c1 = data_pointer[0];
		value.c2 = data_pointer[1];
		value.c3 = data_pointer[2];
		value.c4 = data_pointer[3];
		num_verts = value.i;
		printf("Number of verts: %d\n", num_verts);
		verts = malloc(sizeof(Vertex) * (num_verts + 2));

		data_pointer += 4;
		printf("Reading vertices..\n");
		for(int vertex = 0; vertex < num_verts; vertex++){
			for(int i = 0; i < 6; i++){
				value.c1 = data_pointer[0];
				value.c2 = data_pointer[1];
				value.c3 = data_pointer[2];
				value.c4 = data_pointer[3];

				switch(i % 6){
					case 0: // Vertex X
						verts[vertex].pos.x = value.f;
						break;
					case 1: // Vertex Y
						verts[vertex].pos.y = value.f;
						break;
					case 2: // Vertex Z
						verts[vertex].pos.z = value.f;
						break;
					case 3: // Diffuse
						break;
					case 4: // Texture U
						verts[vertex].uv.x = value.f;
						break;
					case 5: // Texture V
						verts[vertex].uv.y = value.f;
						break;
				}
				data_pointer += 4;
			}
		}

		value.c1 = data_pointer[0];
		value.c2 = data_pointer[1];
		value.c3 = data_pointer[2];
		value.c4 = data_pointer[3];
		unsigned int num_indices = value.i;
		printf("Number of indices: %d\n", num_indices);
		tris = malloc(num_indices * 2 + 2);

		data_pointer += 4;
		printf("Reading indices..\n");
		num_triangles = num_indices / 3;
		for(int triangle = 0; triangle < num_triangles; triangle++){
			for(int index = 0; index < 3; index++){
				value.c1 = data_pointer[0];
				value.c2 = data_pointer[1];
				tris[triangle][index] = value.s1;
				data_pointer += 2;
			}
		}
	}
}

int main(int argc, char *argv[]){

	printf("modl2obj by Pimy (2021)\n\n");

	char path_separator = '/';
	#ifdef _WIN32
		path_separator = '\\';
	#endif

	argv[0][strrchr(argv[0], path_separator) - argv[0]] = 0; // Remove the binary its-self from the path
	path = argv[0];

	for(int i = 1; i < argc; i++){
		#ifdef _WIN32
			ReplaceChars(argv[i], '\\', '/');
		#endif
		if(strrchr(argv[i], '/') != NULL){
			printf("\nConverting '%s':\n", strrchr(argv[i], '/') + 1);
		}else{
			printf("\nConverting '%s':\n", argv[i]);
		}
		ReadFile(argv[i]);
		if(!skip_file){
			ParseData();
			WriteFile(argv[i]);
			printf("Done!\n");
			free(verts);
			free(tris);
			free(mesh_data);
			data_length = 0;
			num_verts = 0;
			num_triangles = 0;
		}
		skip_file = false;
	}

	if(argc == 1){
		printf("To use this program simply drag your .modl file(s) onto modl2obj.exe\nYour new .obj files will be in the same folder as the original .modl file(s)!\n");
	}else{
		printf("\n\nConversions done! You can find your new .obj file(s) in the same folder as the original .modl file(s)\n");
	}

	printf("\n\nPress any key to continue...");
	getchar();

	return 0;
}