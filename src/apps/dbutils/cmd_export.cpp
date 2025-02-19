#include <iostream>

#include <docopt.h>
#include "golpe.h"

#include "events.h"


static const char USAGE[] =
R"(
    Usage:
      export [--since=<since>] [--until=<until>] [--reverse]
)";


void cmd_export(const std::vector<std::string> &subArgs) {
    std::map<std::string, docopt::value> args = docopt::docopt(USAGE, subArgs, true, "");

    uint64_t since = 0, until = MAX_U64;
    if (args["--since"]) since = args["--since"].asLong();
    if (args["--until"]) until = args["--until"].asLong();
    bool reverse = args["--reverse"].asBool();

    Decompressor decomp;

    auto txn = env.txn_ro();

    auto dbVersion = getDBVersion(txn);

    if (dbVersion == 0) throw herr("migration from DB version 0 not supported by this version of strfry");

    uint64_t start = reverse ? until : since;
    uint64_t startDup = reverse ? MAX_U64 : 0;

    exitOnSigPipe();

    env.generic_foreachFull(txn, env.dbi_Event__created_at, lmdb::to_sv<uint64_t>(start), lmdb::to_sv<uint64_t>(startDup), [&](auto k, auto v) {
        if (reverse) {
            if (lmdb::from_sv<uint64_t>(k) < since) return false;
        } else {
            if (lmdb::from_sv<uint64_t>(k) > until) return false;
        }

        auto view = lookupEventByLevId(txn, lmdb::from_sv<uint64_t>(v));

        std::cout << getEventJson(txn, decomp, view.primaryKeyId) << "\n";

        return true;
    }, reverse);
}
