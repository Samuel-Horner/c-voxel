#ifndef TEXT
#define TEXT

#include "cglm/vec3.h"
#include "engine.c"
#include "shader.c"

#include "glad/gl.h"
#include "cglm/cglm.h"

#include <ft2build.h>
#include FT_FREETYPE_H

FT_Library ft_library;
FT_Face ft_face;

typedef struct Character {
    unsigned int texture_id;
    ivec2 size;
    ivec2 bearing;
    unsigned int advance;
} Character;

Character ft_characters[128];

int initFreeType() {
    if (FT_Init_FreeType(&ft_library)) {
        printf("ERROR: error in initliasing FreeType.\n");
        return 0;
    }
    return 1;
}

int setFreeTypeFaceSize(int window_width, int window_height) {
    return FT_Set_Char_Size(ft_face, 0, 16 * 64, window_width, window_height);
}

int initFreeTypeFace(char font[], int window_width, int window_height) {
    int error = FT_New_Face(ft_library, font, 0, &ft_face);
    if (error == FT_Err_Unknown_File_Format){
        printf("ERROR: error when creating font face, unkown file type.\n");
        return 0;
    } else if (error) {
        printf("ERROR: error when initiliasing font face.\n");
        return 0;
    }

    printf("Loaded font: %s\n", font);

    setFreeTypeFaceSize(window_width, window_height);

    return 1;
}

int generateFreeTypeTexture() {
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    printf("Loading characters:\n");

    for (unsigned char c = 0; c < 128; c++) {
        if (FT_Load_Char(ft_face, c, FT_LOAD_RENDER)) {
            printf("ERROR: FreeType failed to load glyph for char %c\n", c);
            continue;
        }
        
        unsigned int texture;

        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            ft_face->glyph->bitmap.width,
            ft_face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            ft_face->glyph->bitmap.buffer
        );
        // set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // now store character for later use
        ft_characters[c].texture_id = texture;
        ft_characters[c].advance = ft_face->glyph->advance.x;
        glm_ivec2_copy((ivec2) {ft_face->glyph->bitmap.width, ft_face->glyph->bitmap.rows}, ft_characters[c].size);
        glm_ivec2_copy((ivec2) {ft_face->glyph->bitmap_left, ft_face->glyph->bitmap_top}, ft_characters[c].bearing);

        printf("%c", c);
    }

    printf("\n");

    glPixelStorei(GL_UNPACK_ALIGNMENT, 0);

    return 1;
}

void freeFreeType() {
    FT_Done_Face(ft_face);
    FT_Done_FreeType(ft_library);
}

int text_quad_indices[] = {
    2, 1, 0,
    3, 2, 0
};

mat4 text_projection;
void textProjectionFunction(unsigned int location) {
    glUniformMatrix4fv(location, 1, GL_FALSE, (float *) text_projection);
}

void updateTextProjection(int window_width, int window_height) {
    glm_ortho(0., window_width, window_height, 0., 0., 1., text_projection);
}

vec3 text_color = GLM_VEC3_ONE_INIT;
void textColorFunction(unsigned int location) {
    glUniform3f(location, text_color[0], text_color[1], text_color[2]);
}

ProgramBundle createTextProgram(int window_width, int window_height) {
    // Fetch vertex and fragment source
    char *text_vertex_source = getShaderSource("./src/shaders/text_vert.glsl");
    if (text_vertex_source == NULL) { printf("Error fecthing text vertex shader.\n"); exit(-1); }

    char *text_fragment_source = getShaderSource("./src/shaders/text_frag.glsl");
    if (text_fragment_source == NULL) { printf("Error fecthing text fragment shader.\n"); exit(-1); }

    // Create program bundle
    ProgramBundle text_program = createProgram(text_vertex_source, text_fragment_source);
 
    // Initialise orthographic projection
    updateTextProjection(window_width, window_height);

    // Bind uniforms
    #define TEXT_UNIFORM_COUNT 2
    char *uniform_names[TEXT_UNIFORM_COUNT] = {"projection", "textColor"};
    UniformFunction uniform_funcs[TEXT_UNIFORM_COUNT] = {textProjectionFunction, textColorFunction};
    bindUniforms(&text_program, uniform_names, uniform_funcs, TEXT_UNIFORM_COUNT);

    return text_program;
}

BufferBundle createTextBuffer() {
    VertexArray verts = { .size = 4 * 4, .values = NULL };
    IndexArray indices = { .size = 6, .values = text_quad_indices};

    unsigned int value_split[] = {4};
    BufferBundle bundle = createVAO(verts, indices, 4, 1, value_split, GL_DYNAMIC_DRAW);

    return bundle;
}

#define CHAR_WIDTH_SCALE 0.65

void renderText(BufferBundle *bundle, ProgramBundle *program, char *text, vec2 pos, float scale) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Cannot use the render function since we need to bind the texture
    glUseProgram(program->programID);
    applyUniforms(program);

    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(bundle->VAO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bundle->EBO);

    do {
        if (!*text) { continue; } 
        Character ch = ft_characters[*text];

        float xpos = pos[0] + ch.bearing[0] * scale;
        float ypos = pos[1] + (ft_characters['H'].bearing[1] - ch.bearing[1]) * scale;
    
        float width = ch.size[0] * scale * CHAR_WIDTH_SCALE;
        float height = ch.size[1] * scale;

        float vertices[4 * 4] = {
            xpos,         ypos + height,   0., 1.,
            xpos,         ypos,            0., 0.,
            xpos + width, ypos,            1., 0.,
            xpos + width, ypos + height,   1., 1., 
        };

        glBindTexture(GL_TEXTURE_2D, ch.texture_id);
        glBindBuffer(GL_ARRAY_BUFFER, bundle->VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * 4 * 4, vertices);

        glDrawElements(GL_TRIANGLES, bundle->length, GL_UNSIGNED_INT, 0);

        pos[0] += (ch.advance >> 6) * scale * CHAR_WIDTH_SCALE;

    } while (*text++);

    glDisable(GL_BLEND);
}

void moveText(float x, float y) {}

#endif
