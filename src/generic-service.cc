#include "quavis/vk/geometry/geometry.h"
#include "quavis/quavis.h"

#include <stdio.h>
#include <argp.h>
#include <signal.h>
#include <unistd.h>

void exithandler(int param) {
  exit(1);
}

/* Argument parsing options */
struct arguments {
  char const *geojson_file;
  char const *cp_shader_1;
  char const *cp_shader_2;
  float max_distance;
  float max_angle;
  int debug_mode;
  int line_mode;
  int timing_mode;
  int disable_geom;
  int disable_tess;
  int render_width;
  int workgroups;
};
static char doc[] = "Runs the generic isovist service on a given input file.";
static char args_doc[] = "";
static struct argp_option options[] = {
  {"geojson_file", 'f', "<path>", 0, "The geojson file that is going to be processed"},
  {"cp_shader_1", 's', "<path>", 0, "The path to the 1st level compute shader"},
  {"cp_shader_2", 't', "<path>", 0, "The path to the 2nd level compute shader"},
  {"max_distance", 'r', "100000", 0, "The maximum visible distance"},
  {"max_angle", 'a', "0.1", 0, "The maximum angle, controls tessellation"},
  {"debug", 'd', "0", 0, "debug-mode=1"},
  {"line", 'l', "0", 0, "line-mode=1"},
  {"timing", 'u', "0", 0, "timing-mode=1"},
  {"disable_geom", 'G', "0", 0, "disable_geom=1"},
  {"disable_tess", 'T', "0", 0, "disable_tess=1"},
  {"width", 'x', "2048", 0, "The rendering width"},
  {"workgroups", 'w', "1024", 0, "The number of workgroups (preferebly the height of the image)"},
  {0}
};
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  struct arguments *args = (arguments*)(state->input);
  switch (key)
    {
    case 'f':
      args->geojson_file = arg;
      break;
    case 's':
      args->cp_shader_1 = arg;
      break;
    case 't':
      args->cp_shader_2 = arg;
      break;
    case 'r':
      args->max_distance = arg ? atof(arg) : 100000;
      break;
    case 'a':
      args->max_angle = arg ? atof(arg) : 0.1;
      break;
    case 'd':
      args->debug_mode = arg ? atoi(arg) : 0;
      break;
    case 'l':
      args->line_mode = arg ? atoi(arg) : 0;
      break;
    case 'u':
      args->timing_mode = arg ? atoi(arg) : 0;
      break;
    case 'G':
      args->disable_geom = arg ? atoi(arg) : 0;
      break;
    case 'T':
      args->disable_tess = arg ? atoi(arg) : 0;
      break;
    case 'x':
      args->render_width = arg ? atoi(arg) : 0;
      break;
    case 'w':
      args->workgroups = arg ? atoi(arg) : 0;
      break;
    case ARGP_KEY_END:
      if (state->arg_num < 0) argp_usage (state);
      break;
    case ARGP_KEY_ARG:
      if (state->arg_num > 0) argp_usage(state);
      //args->host = arg; // old usage where host is required
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

/* Run service using arguments */
int main(int argc, char **argv) {
  struct arguments args;

  /* Default values. */
  args.geojson_file = "input.json";
  args.cp_shader_1 = "shader.comp.spv";
  args.cp_shader_2 = "shader.2.comp.spv";
  args.max_distance = 100000;
  args.max_angle = 0.1;
  args.debug_mode = 0;
  args.line_mode = 0;
  args.timing_mode = 0;

  /* Parse our arguments; every option seen by parse_opt will be
     reflected in arguments. */
  static struct argp argp = { options, parse_opt, args_doc, doc };
  argp_parse(&argp, argc, argv, 0, 0, &args);

  std::vector<quavis::vec3> observation_points;
  float x, y, z;
  while(std::cin >> x >> y >> z) {
    observation_points.push_back(quavis::vec3 { x, y, z });
  }
  quavis::Context* context = new quavis::Context(
    args.cp_shader_1,
    args.cp_shader_2,
    args.debug_mode == 1 ? true : false,
    args.line_mode == 1 ? true : false,
    args.timing_mode,
    args.disable_geom == 1 ? true : false,
    args.disable_tess == 1 ? true : false,
    args.render_width,
    args.workgroups
  );
  std::vector<float> results = context->Parse(args.geojson_file, observation_points, args.max_angle, args.max_distance);

  /* Start the Service */
  signal(SIGINT, exithandler);
}
