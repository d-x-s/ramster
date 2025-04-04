#pragma once

#include <array>
#include <utility>

#include "common.hpp"
#include "tinyECS/components.hpp"
#include "tinyECS/tiny_ecs.hpp"

// System responsible for setting up OpenGL and for rendering all the
// visual entities in the game
class RenderSystem
{
  /**
   * The following arrays store the assets the game will use. They are loaded
   * at initialization and are assumed to not be modified by the render loop.
   *
   * Whenever possible, add to these lists instead of creating dynamic state
   * it is easier to debug and faster to execute for the computer.
   */
  std::array<GLuint, texture_count> texture_gl_handles;
  std::array<ivec2, texture_count> texture_dimensions;

  // Make sure these paths remain in sync with the associated enumerators.
  // Associated id with .obj path
  const std::vector<std::pair<GEOMETRY_BUFFER_ID, std::string>> mesh_paths = {
      std::pair<GEOMETRY_BUFFER_ID, std::string>(GEOMETRY_BUFFER_ID::CHICKEN, mesh_path("chicken.obj"))
      // specify meshes of other assets here
  };

  // Make sure these paths remain in sync with the associated enumerators (see TEXTURE_ASSET_ID).
  const std::array<std::string, texture_count> texture_paths = {

      // Screen Elements
      textures_path("screenElements/menu_title.png"),
      textures_path("screenElements/pause_title.png"),
      textures_path("screenElements/victory_title.png"),
      textures_path("screenElements/defeat_title.png"),
      textures_path("screenElements/menu_text.png"),
      textures_path("screenElements/pause_text.png"),
      textures_path("screenElements/gameover_text.png"),
      textures_path("screenElements/button_lvlUp.png"),
      textures_path("screenElements/button_lvlDown.png"),
      textures_path("screenElements/button_start.png"),
      textures_path("screenElements/button_resume.png"),
      textures_path("screenElements/button_restart.png"),
      textures_path("screenElements/button_mainMenu.png"),
      textures_path("screenElements/button_exit.png"),

      // Screens
      textures_path("screens/main_menu.png"),
      textures_path("screens/test_screen.png"), // This one's the playing screen that we will never use.
      textures_path("screens/pause.png"),
      textures_path("screens/game_ended_screen.png"),

      // Legacy invaders code
      textures_path("invaders/blue_1.png"),
      textures_path("invaders/blue_2.png"),
      textures_path("invaders/blue_3.png"),
      textures_path("invaders/red_1.png"),
      textures_path("invaders/red_2.png"),
      textures_path("invaders/red_3.png"),
      textures_path("invaders/green_1.png"),
      textures_path("invaders/green_2.png"),
      textures_path("invaders/green_3.png"),
      textures_path("invaders/grey_1.png"),
      textures_path("invaders/grey_2.png"),
      textures_path("invaders/grey_3.png"),
      textures_path("towers/tower01.png"),
      textures_path("projectiles/gold_bubble.png"),
      textures_path("projectiles/grapple_point.png"),
      textures_path("projectiles/grapple_outline.png"),
      textures_path("effects/explosion1.png"),
      textures_path("effects/explosion2.png"),
      textures_path("effects/explosion3.png"),
      textures_path("invaders/floater_1.png"),
      textures_path("invaders/floater_2.png"),
      textures_path("invaders/floater_3.png"),

      textures_path("invaders/common_1.png"),
      textures_path("invaders/common_2.png"),
      textures_path("invaders/common_3.png"),
      textures_path("invaders/common_4.png"),
      textures_path("invaders/common_5.png"),

      textures_path("invaders/swarm_1.png"),
      textures_path("invaders/swarm_2.png"),
      textures_path("invaders/swarm_3.png"),
      textures_path("invaders/swarm_4.png"),

      textures_path("invaders/obstacle_1.png"),
      textures_path("invaders/obstacle_2.png"),
      textures_path("invaders/obstacle_3.png"),
      textures_path("invaders/obstacle_4.png"),

      // tiles
    textures_path("tiles/half-ramp-bl.png"),
    textures_path("tiles/half-ramp-br.png"),
    textures_path("tiles/half-ramp-tl.png"),
    textures_path("tiles/half-ramp-tr.png"),
    textures_path("tiles/half-square-2-bottom.png"),
    textures_path("tiles/half-square-2-left.png"),
    textures_path("tiles/half-square-2-right.png"),
    textures_path("tiles/half-square-2-top.png"),
    textures_path("tiles/half-square-bottom.png"),
    textures_path("tiles/half-square-left.png"),
    textures_path("tiles/half-square-right.png"),
    textures_path("tiles/half-square-top.png"),
    textures_path("tiles/smooth-ramp-bl.png"),
    textures_path("tiles/smooth-ramp-br.png"),
    textures_path("tiles/smooth-ramp-tl.png"),
    textures_path("tiles/smooth-ramp-tr.png"),
    textures_path("tiles/square-tile-1.png"),
    textures_path("tiles/square-tile-2.png"),
    textures_path("tiles/tall-ramp-bl.png"),
    textures_path("tiles/tall-ramp-br.png"),
    textures_path("tiles/tall-ramp-tl.png"),
    textures_path("tiles/tall-ramp-tr.png"),
    textures_path("tiles/tesla-trap-1-bottom.png"),
    textures_path("tiles/tesla-trap-1-left.png"),
    textures_path("tiles/tesla-trap-1-right.png"),
    textures_path("tiles/tesla-trap-1-top.png"),

    // tutorial
    textures_path("tutorial/space.png"),
    textures_path("tutorial/move.png"),
    textures_path("tutorial/grapple.png"),
    textures_path("tutorial/destroy.png"),

    // levels
	textures_path("levels/level1.png"),
    textures_path("levels/level2.png"),
	textures_path("levels/level3.png"),
	textures_path("levels/level4.png"),
	textures_path("levels/level5.png"),
    textures_path("levels/level6.png"),
    textures_path("levels/tutorial.png"),
	textures_path("levels/tower.png"),
	textures_path("levels/lab.png"),
	textures_path("levels/under.png"),
	textures_path("levels/snake.png"),
    textures_path("levels/tunnelsmall.png"),
    // <--- Add Next level here !
    textures_path("levels/background.png"),

    // fireball effect frames
    textures_path("fireball_effect/frame_00_delay-0.06s.png"),
    textures_path("fireball_effect/frame_01_delay-0.06s.png"),
	textures_path("fireball_effect/frame_02_delay-0.06s.png"),
    textures_path("fireball_effect/frame_03_delay-0.06s.png"),
	textures_path("fireball_effect/frame_04_delay-0.06s.png"),
	textures_path("fireball_effect/frame_05_delay-0.06s.png"),
	textures_path("fireball_effect/frame_06_delay-0.06s.png"),
	textures_path("fireball_effect/frame_07_delay-0.06s.png"),
	textures_path("fireball_effect/frame_08_delay-0.06s.png"),
	textures_path("fireball_effect/frame_09_delay-0.06s.png"),
	textures_path("fireball_effect/frame_10_delay-0.06s.png"),
	textures_path("fireball_effect/frame_11_delay-0.06s.png"),
  };

  std::array<GLuint, effect_count> effects;
  // Make sure these paths remain in sync with the associated enumerators.
  const std::array<std::string, effect_count> effect_paths = {
      shader_path("coloured"),
      shader_path("egg"),
      shader_path("chicken"),
      shader_path("textured"),
      shader_path("vignette"),
      shader_path("parallax"),
  };

  std::array<GLuint, geometry_count> vertex_buffers;
  std::array<GLuint, geometry_count> index_buffers;
  std::array<Mesh, geometry_count> meshes;

public:

  // Initialize the window
  bool init(GLFWwindow *window);

  template <class T>
  void bindVBOandIBO(GEOMETRY_BUFFER_ID gid, std::vector<T> vertices, std::vector<uint16_t> indices);

  void initializeGlTextures();

  void initializeGlEffects();

  void initializeGlMeshes();

  Mesh &getMesh(GEOMETRY_BUFFER_ID id) { return meshes[(int)id]; };

  void initializeGlGeometryBuffers();

  // Initialize the screen texture used as intermediate render target
  // The draw loop first renders to this texture, then it is used for the vignette shader
  bool initScreenTexture();

  // Destroy resources associated to one or all entities created by the system
  ~RenderSystem();

  // Draw all entities
  void draw(float elapsed_ms, bool game_active);

  mat3 createProjectionMatrix();

  Entity get_screen_state_entity() { return screen_state_entity; }

  // Window resizing
  void resizeScreenTexture(int width, int height);

  int screen_viewport_x = 0, screen_viewport_y = 0;
  int screen_viewport_w = 1200, screen_viewport_h = 900;

private:
  // Internal drawing functions for each entity type
  void drawGridLine(Entity entity, const mat3 &projection);
  void drawLine(Entity entity, const mat3 &projection);
  void drawTexturedMesh(Entity entity, const mat3 &projection, float elapsed_ms, bool game_active);
  void drawToScreen();

  // Window handle
  GLFWwindow *window;

  // Screen texture handles
  GLuint frame_buffer;
  GLuint off_screen_render_buffer_color;
  GLuint off_screen_render_buffer_depth;

  Entity screen_state_entity;

};

bool loadEffectFromFile(
    const std::string &vs_path, const std::string &fs_path, GLuint &out_program);
