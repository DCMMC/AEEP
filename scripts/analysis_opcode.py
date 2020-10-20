import sys
import re
import os
import json

import time
import numpy as np

from slither.core.variables.state_variable import StateVariable
from slither.slither import Slither
from slither.slithir.operations.high_level_call import HighLevelCall
from slither.slithir.operations.index import Index
from slither.slithir.variables.reference import ReferenceVariable
from slither.slithir.variables.temporary import TemporaryVariable
import slither.analyses.evm.convert as SourceToEVM

global taint_opcode
taint_opcode = []


def visit_node(node, visited):
    if node in visited or node is None:
        return

    print('  Node expression: {}'.format(node.expression))

    flag = False

    visited += [node]
    taints = node.function.slither.context[KEY]

    refs = {}
    for ir in node.irs:
        print('    IR: {}'.format(ir))

        if isinstance(ir, Index):
            refs[ir.lvalue] = ir.variable_left

        if isinstance(ir, Index):
            read = [ir.variable_left]
        else:
            read = ir.read


        try:
            if any(var_read in taints for var_read in read):
                flag = True
                taints += [ir.lvalue]
                lvalue = ir.lvalue
                while isinstance(lvalue, ReferenceVariable):
                    taints += [refs[lvalue]]
                    lvalue = refs[lvalue]
        except Exception:
            pass


    if flag:
        node_ins = SourceToEVM.get_evm_instructions(node)
        for ins in node_ins:
            taint_opcode.append(str(ins))

    taints = [v for v in taints if not isinstance(v, (TemporaryVariable, ReferenceVariable))]

    node.function.slither.context[KEY] = list(set(taints))

    for son in node.sons:
        try:
          visit_node(son, visited)
        except:
          pass


def clean_taint_opcode():
    for i in range(len(taint_opcode)):
        searchObj = re.search("[0-9]", taint_opcode[i])
        if searchObj:
            taint_opcode[i] = taint_opcode[i][0:searchObj.span()[0]]

    return list(set(taint_opcode))


def check_call(func, taints):
    for node in func.nodes:
        for ir in node.irs:
            if isinstance(ir, HighLevelCall):
                if ir.destination in taints:
                    print('Call to tainted address found in {}'.format(function.name))


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print('ERROR: EXAMPLE: python analysis_opcode.py ../example/xxx.sol')
        exit(-1)

    flag = True if len(sys.argv) >= 3 and sys.argv[2] == '-m' else False
    epoch = 3 if flag is True else 1

    time1_record = []
    time2_record = []
    for i in range(epoch):
        taint_opcode = []
        start_time = time.time()
        slither = Slither(sys.argv[1])
        KEY = 'TAINT'
        initial_taint = []
        prev_taints = []
        epoch = 0
        slither.context[KEY] = initial_taint
        while set(prev_taints) != set(slither.context[KEY]) or epoch == 0:
            prev_taints = slither.context[KEY]
            epoch += 1
            for contract in slither.contracts:
                for function in contract.functions:
                    print('Function {}'.format(function.name))
                    slither.context[KEY] = list(set(slither.context[KEY] + function.parameters))
                    visit_node(function.entry_point, [])
                    print('All variables tainted : {}'.format([str(v) for v in slither.context[KEY]]))
                    print()

        print('All variables tainted : {}'.format([str(v) for v in slither.context[KEY]]))
        taint_opcode = clean_taint_opcode()
        print('\nAll tainted opcodes: {}'.format(taint_opcode))

        for function in contract.functions:
            check_call(function, slither.context[KEY])
        end_time = time.time()
        time1_record.append(end_time - start_time)


        start_time = time.time()
        # create enclave_ecalls.cpp
        enclave_ecalls_1 = open('./enclave_ecalls/enclave_ecalls_1.cpp', 'r', encoding='utf-8').read()
        enclave_ecalls_2 = open('./enclave_ecalls/enclave_ecalls_2.cpp', 'r', encoding='utf-8').read()

        eop_oop_list = [str[:-4] for str in os.listdir('./eop')]
        taint_opcode = [str.lower() for str in taint_opcode]
        for op in eop_oop_list:
            if op in taint_opcode:
                content = open('./eop/{}.cpp'.format(op), 'r', encoding='utf-8').read()
            else:
                content = open('./oop/{}.cpp'.format(op), 'r', encoding='utf-8').read()
            enclave_ecalls_1 = enclave_ecalls_1 + '\n\n' + content
        enclave_ecalls = enclave_ecalls_1 + '\n\n' + enclave_ecalls_2

        with open('../enclave/enclave_ecalls.cpp', 'w', encoding='utf-8') as f:
            f.write(enclave_ecalls)
            print('\nenclave_ecalls.cpp write done')
        end_time = time.time()
        time2_record.append(end_time - start_time)
    if flag:
        print('taint analysis time: {} {}'.format(np.mean(time1_record), np.std(time1_record)))
        print('partition time: {} {}'.format(np.mean(time2_record), np.std(time2_record)))
