#include <luciconnect/luciconnect.h>
#include "quavis/vk/geometry/geometry.h"
#include "quavis/quavis.h"

#include <signal.h>

INITIALIZE_EASYLOGGINGPP

class GenericIsovistService : luciconnect::Node {

public:
  GenericIsovistService(std::shared_ptr<luciconnect::Connection> connection) : luciconnect::Node(connection) {}

  void Run() {
    this->Connect();
    this->SendRun(0, "RemoteRegister", this->register_message_);

    while(1);
  }

protected:
  const json register_message_ = {
    {"serviceName", "GenericIsovistService"},
    {"description", "Returns the Isovist of a given scenario"},
    {"qua-view-compliant", true},
    {"inputs", {
      {"ScID", "number"},
      {"mode", "string"},
      {"points", "attachment"},
      {"alpha", "number"}
    }},
    {"outputs", {
      {"units", "string"},
      {"values", "string"}
    }},
    {"constraints", {
      {"mode", {"points", "objects", "scenario", "new"}},
      {"alpha", {
        {"integer", false},
        {"min", 0.01},
        {"max", 1.5},
        {"def", 0.05}
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
    this->SendRun(13371, "scenario.geojson.Get", {{"ScID", inputs["ScID"]}});
  };


  void HandleResult(int64_t callId, json result, std::vector<luciconnect::Attachment*> attachments) {
    if (callId == 13371) {
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
          quavis::Context* context = new quavis::Context();
          std::vector<float> results = context->Parse(geojson, this->current_points);
          json result = {
            {"units", "m3"},
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
};

void exithandler(int param) {
  exit(1);
}

int main(int argc, char **argv) {
  signal(SIGINT, exithandler);

  std::shared_ptr<luciconnect::Connection> connection = std::make_shared<luciconnect::Connection>("localhost", 7654);
  GenericIsovistService* service = new GenericIsovistService(connection);
  service->Run();
}
