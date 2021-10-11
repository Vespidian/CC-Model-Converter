#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "shared.h"

// Per model variables
bool skip_file = false;
char *path;

// Mesh data and types
char *mesh_data;
unsigned int data_length = 0;

// Individual position data
unsigned int num_positions = 0;
Vector3 *positions;

// Individual UV data
unsigned int num_uvs = 0;
Vector2 *uvs;


// Final product to be written to .modl file
Vector3 bounds[2];

unsigned int num_verts;
Vertex *verts;
 
unsigned int num_triangles = 0;
Triangle *tris;


bool CompareVert(Vertex v1, Vertex v2){
	if(
		(v1.pos.x == v2.pos.x) && 
		(v1.pos.y == v2.pos.y) && 
		(v1.pos.z == v2.pos.z) &&
		(v1.uv.x == v2.uv.x) &&
		(v1.uv.y == v2.uv.y)
	){
		return true;
	}
	return false;
}

int FindVert(Vertex *v){
	for(int i = 0; i < num_verts; i++){
		if(CompareVert(*v, verts[i])){
			return i;
		}
	}
	return -1;
}

// Takes input in the form: 'int/int' ie: '127/4'
unsigned int StrToVert(char *str){
	unsigned int pos_id = strtol(str, NULL, 10) - 1;
	unsigned int uv_id = strtol(strchr(str, '/') + 1, NULL, 10) - 1;

	Vertex tmp_vert;
	tmp_vert.pos = positions[pos_id];
	tmp_vert.uv = uvs[uv_id];

	int v_index = FindVert(&tmp_vert);
	if(v_index == -1){ // Vert not found
		verts = realloc(verts, sizeof(Vertex) * (num_verts + 1));
		verts[num_verts] = tmp_vert;
		v_index = num_verts;
		num_verts++;
	}

	return v_index;
}

void ReadFile(char *file){
	if(strcmp(file + strlen(file) - 4, ".obj") == 0){
		// Open the file
		FILE *obj_file;
		obj_file = fopen(file, "rb");
		if(obj_file == NULL){
			printf("Error opening file!\n");
			skip_file = true;
			return;
		}
		
		// Find length of the source file
		fseek(obj_file, 0, SEEK_END);
		int file_size = ftell(obj_file);
		fseek(obj_file, 0, SEEK_SET);
		data_length = file_size;

		// Allocate space for data
		mesh_data = malloc(file_size + 2);

		// Read to data array
		size_t error_check = fread(mesh_data, 1, file_size, obj_file);
		if(error_check != file_size){
			printf("Error reading file!\n");
			skip_file = true;
			return;
		}

		// Close the file
		fclose(obj_file);
	}else{
		printf("Skipping non .obj file (This program can only convert .obj file into .modl files)\n");
		skip_file = true;
	}
}

void WriteFile(char *file){
	file[strlen(file) - 4] = 0;
	if(strrchr(file, '/') != NULL){
		file = strrchr(file, '/') + 1;
	}

	char file_path[64];
	sprintf(file_path, "%s.modl", file);

	errno = 0;
	FILE *modl_file;
	modl_file = fopen(file_path, "wb");
	if(modl_file == NULL){
		printf("error: Could not write file (not enough permissions)\n");
		return;
	}

	/**
	 *  MODL Format:
	 *  (24 bytes) 	bounding box: 
	 * 		(4 bytes)	x_min
	 * 		(4 bytes)	y_min
	 * 		(4 bytes)	z_min
	 * 		(4 bytes)	x_max
	 * 		(4 bytes)	y_max
	 * 		(4 bytes)	z_max
	 *  (1  byte ) 	delimiting 0
	 *  (4  bytes) 	number of vertices
	 * 	Loop through vertices:
	 *  (24 bytes) 	vertex: (all vertex values are 4 bytes)
	 * 		(4 bytes)	x		
	 * 		(4 bytes)	y		
	 * 		(4 bytes)	z		
	 * 		(4 bytes)	diffuse	(usually 0xffffffff)
	 * 		(4 bytes)	u		
	 * 		(4 bytes)	v		
	 *  (4 bytes) 	number of indices (3 * num_triangles)
	 *  Loop through indices:
	 *  (6 bytes)	index: (each value is 2 bytes (short)) (indexes tell cc what vertices to use to create triangles)
	 * 		(2 bytes)	v1
	 * 		(2 bytes)	v2
	 * 		(2 bytes)	v3
	 *  (8 bytes)	zero.. i dunno, looks like theres an 8 byte delimiter at the end thats always filled with 0
	 */

	int value = 0;

	// Bounding box
	fwrite(&bounds[1].v, sizeof(float), 3, modl_file);
	fwrite(&bounds[0].v, sizeof(float), 3, modl_file);
	fwrite(&value, 1, 1, modl_file);

	// Vertices
	fwrite(&num_verts, sizeof(int), 1, modl_file);
	for(int i = 0; i < num_verts; i++){
		fwrite(&verts[i].pos.v, sizeof(float), 3, modl_file); 	// Position
		value = 0xffffffff;
		fwrite(&value, sizeof(float), 1, modl_file);			// Diffuse
		fwrite(&verts[i].uv.v, sizeof(float), 2, modl_file);	// UV
	}

	// Indexed triangles / faces
	value = num_triangles * 3;
	fwrite(&value, sizeof(int), 1, modl_file);
	for(int i = 0; i < num_triangles; i++){
		fwrite(tris[i], 2, 3, modl_file);
	}
	value = 0;
	fwrite(&value, 4, 1, modl_file);
	fwrite(&value, 4, 1, modl_file);

	fclose(modl_file);

}

void ParseData(){
	char *ptr = mesh_data;

	while(ptr < mesh_data + data_length && !skip_file){
		unsigned int line_len = strchr(ptr, '\n') - ptr;

		switch(ptr[0]){
			case 'v':
				if(ptr[1] == 't'){ // Texture coordinates
					uvs = realloc(uvs, sizeof(Vector2) * (num_uvs + 1));
					sscanf(ptr + 3, "%f %f", &uvs[num_uvs].x, &uvs[num_uvs].y);
					num_uvs++;
				}else if(ptr[1] == ' '){ // Vertex positions
					positions = realloc(positions, sizeof(Vector3) * (num_positions + 1));
					sscanf(ptr + 2, "%f %f %f", &positions[num_positions].x, &positions[num_positions].y, &positions[num_positions].z);

					// Check bounds
					for(int i = 0; i < 3; i++){
						if(positions[num_positions].v[i] > bounds[0].v[i]){
							bounds[0].v[i] = positions[num_positions].v[i];
						}else if(positions[num_positions].v[i] < bounds[1].v[i]){
							bounds[1].v[i] = positions[num_positions].v[i];
						}
					}

					num_positions++;
				}
				break;
			case 'f':; // Triangles
					// READING FACES REMEMBER ARRAYS ARE STARTING AT 1 HERE (in obj files)
					// If we find // in any of the vertices in a face, we know we cannot use this obj since it doesnt have texture coordinates
					// Or if there are no slashes (/), we dont have texture data either
					// Then we make sure that there arent any more than 6 forward slashes (/) on one line (v/vn/vt * 3) this makes sure everything is triangulated
					// If there are greater than 3 (/) on one line, we know we have to deal with normals in the data as well
					char vert_strs[3][64];
					int slashes;
					if((slashes = CountChars(ptr, '/', line_len)) <= 6){ // Only using triangles
						if(!strnstr(ptr, "//", line_len) && slashes != 0){ // Make sure we have texture coordinate data
							// if(slashes >= 6){
								sscanf(ptr + 2, "%s %s %s", vert_strs[0], vert_strs[1], vert_strs[2]);
								unsigned int v1, v2, v3;
								v1 = StrToVert(vert_strs[0]);
								v2 = StrToVert(vert_strs[1]);
								v3 = StrToVert(vert_strs[2]);

								tris = realloc(tris, sizeof(Triangle) * (num_triangles + 1));
								tris[num_triangles][0] = v1;
								tris[num_triangles][1] = v2;
								tris[num_triangles][2] = v3;
								num_triangles++;
							// }
						}else{ // Set uv coordinates to 0
							printf("error: Cannot read files without texture coordinate (uv) data!\n");
							skip_file = true;
						}
					}else{ // Only read the first 3 vertices
						printf("error: Cannot read non triangulated 3d models\n");
						skip_file = true;
					}
				break;
			default: // Comments and anything else that we dont need
				break;
		}

		ptr = strchr(ptr, '\n') + 1;
	}
}

int main(int argc, char *argv[]){

	printf("obj2modl by Pimy (2021)\n\n");

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
			free(positions);
			free(uvs);
			free(verts);
			free(tris);
			free(mesh_data);
			data_length = 0;
			num_positions = 0;
			num_uvs = 0;
			num_verts = 0;
			num_triangles = 0;
		}
		skip_file = false;
	}

	if(argc == 1){
		printf("To use this program simply drag your .obj file(s) onto obj2modl.exe\nYour new .modl file(s) will be in the same folder as the original .obj file(s)!\n");
		printf("\nSome useful tips for generating models to be converted:\n");
		printf("- Make sure your faces are triangulated\n- Set -Z to up\n- If you do want to texture things (I dont recommend it) remember that you need to vertically flip your end result image before placing it into the texture atlas for cc to properly use it\n");
	}else{
		printf("\n\nConversions done! You can find your new .modl file(s) in the same folder as the original .obj file(s)\n");
	}

	printf("\n\nPress any key to continue...");
	getchar();
}