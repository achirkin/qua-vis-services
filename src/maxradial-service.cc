#include <luciconnect/luciconnect.h>
#include "quavis/vk/geometry/geometry.h"
#include "quavis/quavis.h"

#include <stdio.h>
#include <argp.h>
#include <signal.h>
#include <unistd.h>
#include <easylogging++.h>

INITIALIZE_EASYLOGGINGPP

class GenericIsovistService : public luciconnect::quaview::Service {

public:
  GenericIsovistService(std::shared_ptr<luciconnect::Connection> connection) : luciconnect::quaview::Service(connection) {}

  void Run() override {
    this->Connect();
    this->SendRun(0, "RemoteRegister", this->register_message_);

    while(1) {
      usleep(50);
    }
  }
  
  std::string GetName() override {
    return register_message_["serviceName"];
  }

  std::string GetDescription() override {
    return register_message_["description"];
  }

  std::string GetUnit() {
    return register_message_["outputs"]["units"];
  }

  json GetInputs() override {
    return register_message_["inputs"];
  }

  json GetConstraints() override {
    return register_message_["constraints"];
  }

  bool SupportsPointMode() override {
    return register_message_["constraints"]["mode"];
  }

  // TODO Move computation from HandleRun if its necessary
  std::vector<float>
  ComputeOnPoints(std::vector<luciconnect::vec3> scenario_triangles, std::vector<luciconnect::vec3> points,
                  json inputs) override {
    return *new std::vector<float>{};
  }
protected:
  const json register_message_ = {
    {"serviceName", "quavis-isovist-maxradial"},
    {"description", "Returns the Isovist of a given scenario"},
    {"qua-view-compliant", true},
    {"inputs", {
      {"ScID", "number"},
      {"mode", "string"},
      {"points", "attachment"},
      {"alpha_max", "number"},
      {"r_max", "number"}
    }},
    {"outputs", {
      {"units", "string"},
      {"values", "string"}
    }},
    {"constraints", {
      {"mode", {"points", "objects", "scenario", "new"}},
      {"alpha_max", {
        {"integer", false},
        {"min", 0.01},
        {"max", 1.5},
        {"def", 0.1}
      }},
      {"r_max", {
        {"integer", false},
        {"min", 1},
        {"max", 100000},
        {"def", 5000}
      }}
    }},
    {"exampleCall", {
      {"run", "GenericIsovistService"},
      {"callId", 4386},
      {"mode", "points"},
      {"attachment", {
        {"length", 512},
        {"position", 1},
        {"checksum", "abc"}
      }}
    }}
  };

  void HandleRun(int64_t callId, std::string serviceName, json inputs, std::vector<luciconnect::Attachment*> attachments) {
    this->clientCallId = callId;

    luciconnect::Attachment atc = *attachments[0];

    quavis::vec3* raw = (quavis::vec3*)atc.data;
    this->current_points = std::vector<quavis::vec3>(raw, raw + attachments[0]->size / sizeof(quavis::vec3));
    this->r_max = inputs["r_max"];
    this->alpha_max = inputs["alpha_max"];
    this->SendRun(13372, "scenario.geojson.Get", {{"ScID", inputs["ScID"]}});
  };


  void HandleResult(int64_t callId, json result, std::vector<luciconnect::Attachment*> attachments) {
    if (callId == 13372) {
      if (result.count("registeredName") > 0) {
        // registered
        /*
        std::vector<quavis::vec3> points = {{0,0,0}, {1,1,1}, {2,2,2}};
        luciconnect::Attachment testattachment = luciconnect::Attachment {points.size()*sizeof(quavis::vec3), (const char*)points.data(), "format", "name"};
        this->HandleRun(1, "test", (json){{"ScID", 2}}, {&testattachment});*/
      }
      else {
        if (result.count("geometry_output") > 0) {
          // got scenario
          std::string geojson = result["geometry_output"]["geometry"].dump();
          quavis::Context* context = new quavis::Context("maxradial");
          std::vector<float> results = context->Parse(geojson, this->current_points, this->alpha_max, this->r_max);
          json result = {
            {"units", "m"},
            {"mode", "points"}
          };
          float* raw = results.data();
          luciconnect::Attachment atc {results.size()*sizeof(float), (const char*)raw, "Float32Array", "values"};
          std::vector<luciconnect::Attachment*> atcs = {&atc};
          this->SendResult(this->clientCallId, result, atcs);
        }
      }
    }
    else {
      std::cout << result << std::endl;
    }
  };


  void HandleCancel(int64_t callId) {};

  void HandleProgress(int64_t callId, int64_t percentage, std::vector<luciconnect::Attachment*> attachments, json intermediateResult) {};

  void HandleError(int64_t callId, std::string error) {
    std::cout << error << std::endl;
  };

private:
  int64_t clientCallId = 0;
  std::vector<quavis::vec3> current_points = {};
  float r_max;
  float alpha_max;
};

void exithandler(int param) {
  exit(1);
}

void run_service(GenericIsovistService* service, int retries) {
  time_t timestamp = std::time(NULL);
  try {
    LOG(INFO) << "Starting Service";
    service->Run();
  }
  catch (const char* what) {
    LOG(WARNING) << "An error occurred: " << what;
    LOG(INFO) << "Trying to reestablish the connection in 1 seconds.";
    usleep(1000000);
    if (std::time(NULL) - timestamp < retries) {
      run_service(service, --retries);
    }
    else {
      LOG(ERROR) << "Number of retries exceeded.";
      exit(-1);
    }
  }
}

/* Argument parsing options */
struct arguments { char const *host; int port; int loglevel; int retries;};
static char doc[] = "Runs the generic isovist service until terminated.";
static char args_doc[] = "";
static struct argp_option options[] = {
  {"host", 'h', "localhost", 0, "The host address of Luci"},
  {"port", 'p', "7654", 0, "The port of Luci"},
  {"loglevel", 'l', "2", 0, "The loglevel\n0: all, 1: debug, 2: info, 3: warning, 4: error"},
  {"retries", 'r', "5", 0, "The number of retries when the connection could not be established or has ended unexpectedly. The service performs one retry per second."},
  {0}
};
static error_t parse_opt (int key, char *arg, struct argp_state *state) {
  struct arguments *args = (arguments*)(state->input);
  switch (key)
    {
    case 'p':
      args->port = arg ? atoi(arg) : 7654;
      break;
    case 'h':
      args->host = arg;
      break;
    case 'l':
      args->loglevel = arg ? atoi(arg) : 2;
      break;
    case 'r':
      args->retries = arg ? atoi(arg) : 5;
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
  args.host = "localhost";
  args.port = 7654;
  args.loglevel = 2;
  args.retries = 5;

  /* Parse our arguments; every option seen by parse_opt will be
     reflected in arguments. */
  static struct argp argp = { options, parse_opt, args_doc, doc };
  argp_parse(&argp, argc, argv, 0, 0, &args);

  /* Configure logging according to command line */
  el::Configurations defaultConf;
  defaultConf.setToDefault();
  switch (args.loglevel) {
    case 4:
      defaultConf.set(el::Level::Warning, el::ConfigurationType::Enabled, "false");
    case 3:
      defaultConf.set(el::Level::Info, el::ConfigurationType::Enabled, "false");
    case 2:
      defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
    case 1:
      defaultConf.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
      break;
    default:
      break;
  }
  el::Loggers::reconfigureLogger("default", defaultConf);

  /* Start the Service */
  signal(SIGINT, exithandler);
  std::shared_ptr<luciconnect::Connection> connection = std::make_shared<luciconnect::Connection>(args.host, args.port);
  GenericIsovistService* service = new GenericIsovistService(connection);
  run_service(service, args.retries);
}
