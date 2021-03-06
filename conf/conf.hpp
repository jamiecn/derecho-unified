#ifndef CONF_HPP
#define CONF_HPP

#include <atomic>
#include <conf/getpot/GetPot>
#include <getopt.h>
#include <inttypes.h>
#include <map>
#include <memory>
#include <stdio.h>
#include <unistd.h>

namespace derecho {

#define CONF_ENTRY_INTEGER(name, section, string)

/** The single configuration file for derecho **/
class Conf {
private:
  // Configuration Table:
  // config name --> default value
#define CONF_DERECHO_LEADER_IP "DERECHO/leader_ip"
#define CONF_DERECHO_LEADER_GMS_PORT "DERECHO/leader_gms_port"
#define CONF_DERECHO_LOCAL_ID "DERECHO/local_id"
#define CONF_DERECHO_LOCAL_IP "DERECHO/local_ip"
#define CONF_DERECHO_GMS_PORT "DERECHO/gms_port"
#define CONF_DERECHO_RPC_PORT "DERECHO/rpc_port"
#define CONF_DERECHO_SST_PORT "DERECHO/sst_port"
#define CONF_DERECHO_RDMC_PORT "DERECHO/rdmc_port"
#define CONF_DERECHO_MAX_PAYLOAD_SIZE "DERECHO/max_payload_size"
#define CONF_DERECHO_MAX_SMC_PAYLOAD_SIZE "DERECHO/max_smc_payload_size"
#define CONF_DERECHO_BLOCK_SIZE "DERECHO/block_size"
#define CONF_DERECHO_WINDOW_SIZE "DERECHO/window_size"
#define CONF_DERECHO_TIMEOUT_MS "DERECHO/timeout_ms"
#define CONF_DERECHO_RDMC_SEND_ALGORITHM "DERECHO/rdmc_send_algorithm"
#define CONF_RDMA_PROVIDER "RDMA/provider"
#define CONF_RDMA_DOMAIN "RDMA/domain"
#define CONF_RDMA_TX_DEPTH "RDMA/tx_depth"
#define CONF_RDMA_RX_DEPTH "RDMA/rx_depth"
#define CONF_PERS_FILE_PATH "PERS/file_path"
#define CONF_PERS_RAMDISK_PATH "PERS/ramdisk_path"

  std::map<const std::string, std::string> config = {
      // [DERECHO]
      {CONF_DERECHO_LEADER_IP, "127.0.0.1"},
      {CONF_DERECHO_LEADER_GMS_PORT, "23580"},
      {CONF_DERECHO_LOCAL_ID, "0"},
      {CONF_DERECHO_LOCAL_IP, "127.0.0.1"},
      {CONF_DERECHO_GMS_PORT, "23580"},
      {CONF_DERECHO_RPC_PORT, "28366"},
      {CONF_DERECHO_SST_PORT, "37683"},
      {CONF_DERECHO_RDMC_PORT, "31675"},
      {CONF_DERECHO_MAX_PAYLOAD_SIZE, "10240"},
      {CONF_DERECHO_MAX_SMC_PAYLOAD_SIZE, "10240"},
      {CONF_DERECHO_BLOCK_SIZE, "1048576"},
      {CONF_DERECHO_WINDOW_SIZE, "16"},
      {CONF_DERECHO_TIMEOUT_MS, "1"},
      {CONF_DERECHO_RDMC_SEND_ALGORITHM, "binomial_send"},
      // [RDMA]
      {CONF_RDMA_PROVIDER, "sockets"},
      {CONF_RDMA_DOMAIN, "eth0"},
      {CONF_RDMA_TX_DEPTH, "256"},
      {CONF_RDMA_RX_DEPTH, "256"},
      // [PERS]
      {CONF_PERS_FILE_PATH, ".plog"},
      {CONF_PERS_RAMDISK_PATH, "/dev/shm/volatile_t"}};

public:
  // the option for parsing command line with getopt(not GetPot!!!)
  static struct option long_options[];

public:
  /** Constructor:
   *  Conf can read configure from multiple sources
   *  - the command line argument has the highest priority, then,
   *  - the configuration files
   *  - the default values.
   **/
  Conf(int argc, char *argv[], GetPot *getpotcfg = nullptr) noexcept {
    // 1 - load configuration from configuration file
    if (getpotcfg != nullptr) {
      for (std::map<const std::string, std::string>::iterator it =
               this->config.begin();
           it != this->config.end(); it++) {
        this->config[it->first] = (*getpotcfg)(it->first, it->second);
      }
    }
    // 2 - load configuration from the command line
    int c;
    while (1) {
      int option_index = 0;

      c = getopt_long(argc, argv, "", long_options, &option_index);
      if (c == -1) {
        break;
      }

      switch (c) {
      case 0:
        this->config[long_options[option_index].name] = optarg;
        break;

      case '?':
        break;

      default:
        std::cerr << "ignore unknown commandline code:" << c << std::endl;
      }
    }
  }
  /** get configuration **/
  const std::string &getString(const std::string &key) const {
    return this->config.at(key);
  }
  const int16_t getInt16(const std::string &key) const {
    return (const int16_t)std::stoi(this->config.at(key));
  }
  const uint16_t getUInt16(const std::string &key) const {
    return (const uint16_t)std::stoi(this->config.at(key));
  }
  const int32_t getInt32(const std::string &key) const {
    return (const int32_t)std::stoi(this->config.at(key));
  }
  const uint32_t getUInt32(const std::string &key) const {
    return (const uint32_t)std::stoi(this->config.at(key));
  }
  const int64_t getInt64(const std::string &key) const {
    return (const int64_t)std::stoll(this->config.at(key));
  }
  const uint64_t getUInt64(const std::string &key) const {
    return (const uint64_t)std::stoll(this->config.at(key));
  }
  const float getFloat(const std::string &key) const {
    return (const float)std::stof(this->config.at(key));
  }
  const double getDouble(const std::string &key) const {
    return (const float)std::stod(this->config.at(key));
  }
  // Initialize the singleton from the command line and the configuration file.
  // The command line has higher priority than the configuration file
  // The process we find the configuration file:
  // 1) if conf_file is not null, use it, otherwise,
  // 2) try DERECHO_CONF_FILE environment, otherwise,
  // 3) use "derecho.cfg" at local, otherwise,
  // 4) use all default settings.
  // Note: initialize will be called on the first 'get()' if it is not called
  // before.
  static void initialize(int argc, char *argv[],
                         const char *conf_file = nullptr);
  static const Conf *get() noexcept;

private:
  // singleton
  static std::unique_ptr<Conf> singleton;
  static std::atomic<uint32_t> singleton_initialized_flag;
};

// helpers
const std::string &getConfString(const std::string &key);
const int16_t getConfInt16(const std::string &key);
const uint16_t getConfUInt16(const std::string &key);
const int32_t getConfInt32(const std::string &key);
const uint32_t getConfUInt32(const std::string &key);
const int64_t getConfInt64(const std::string &key);
const uint64_t getConfUInt64(const std::string &key);
const float getConfFloat(const std::string &key);
const double getConfDouble(const std::string &key);
} // namespace derecho
#endif // CONF_HPP
