create_clock -name { TCK } \
-period 166.67 \
-waveform { 0 83.33 } \
[ get_ports { TCK } ]

# PLL_GEN_CLK is the name applied to the "create_generated_clock" constraint derived
# from your Top Level timing in Constraint Manager

set_false_path -from [ get_clocks { TCK } ] \
-to [ get_clocks { PLL_GEN_CLK } ]
set_false_path -from [ get_clocks { PLL_GEN_CLK } ] \
-to [ get_clocks { TCK } ]