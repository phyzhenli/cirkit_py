import cirkit_py as ckt

ckt.run('read_blif -a ../benchmarks/EPFL/adder.blif')
ckt.run('read_blif -m ../benchmarks/EPFL/adder.blif')
ckt.run('miter -a 0 -m 0; equivalence_checking')

bench = '../benchmarks/EPFL/div.blif'
ckt.run('read_genlib mcnc.genlib; ps')

ckt.run(f'read_blif -a {bench}; ps')
res = ckt.run('aig_script; ps')
print('aig_script', res)
ckt.run('lut_mapping; ps')
ckt.run('ps -a')
ckt.run('map; ps')
ckt.run('ps -a')

ckt.run(f'read_blif -m {bench}; ps')
res = ckt.run('mig_script; ps')
print('mig_script', res)
ckt.run('lut_mapping; ps')
ckt.run('ps -m')
ckt.run('map; ps')
ckt.run('ps -m')

ckt.run(f'read_blif -m {bench}; ps')
res = ckt.run('mig_script2; ps')
print('mig_script2', res)
ckt.run('lut_mapping; ps')
ckt.run('ps -m')
ckt.run('map; ps')
ckt.run('ps -m')

ckt.run(f'read_blif -x {bench}; ps')
res = ckt.run('xmg_script; ps')
print('xmg_script', res)
ckt.run('lut_mapping; ps')
ckt.run('ps -x')
ckt.run('map; ps')
ckt.run('ps -x')

ckt.run(f'read_blif --xag {bench}; ps')
res = ckt.run('xag_script; ps')
print('xag_script', res)
ckt.run('lut_mapping; ps')
ckt.run('ps --xag')
ckt.run('map; ps')
ckt.run('ps --xag')

