//
// Created by kier on 2018/10/31.
//

#ifndef TENSORSTACK_MODULE_MENU_H
#define TENSORSTACK_MODULE_MENU_H

#include "module.h"
#include "utils/ctxmgr.h"

namespace ts {
    namespace bubble {
        /**
         * get Parameter node
         * @param name Node name
         * @return new Node belonging to context-Graph
         * @note Must call `ts::ctx::bind<Graph>` to bind context firstly
         */
        Node param(const std::string &name);

        /**
         * get Parameter node
         * @param name Node name
         * @param op_name Operator name
         * @param inputs Input nodes
         * @return new Node belonging to context-Graph
         * @note Must call `ts::ctx::bind<Graph>` to bind context firstly
         */
        Node op(const std::string &name, const std::string &op_name, const std::vector<Node> &inputs);

        /**
         * get Parameter node
         * @param name Node name
         * @param value the data value
         * @return new Node belonging to context-Graph
         * @note Must call `ts::ctx::bind<Graph>` to bind context firstly
         */
        Node data(const std::string &name, const Tensor &value);
    }
}


#endif //TENSORSTACK_MODULE_MENU_H