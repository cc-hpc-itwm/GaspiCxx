#add executable called "helloDemo" that is built from the source files
# "demo.cxx" and "demo_b.cxx". The extensions are automatically found.
add_executable (1D-wave
                  1D-wave.cpp )

add_executable (perf-measurement
		  perf-measurement.cpp)

# Link the executable to the Hello library. Since the Hello library has
# public include directories we will use those link directories when building
# helloDemo

target_link_libraries (1D-wave LINK_PUBLIC 
			   GaspiCxx                            
			   pthread
			   rt)

target_link_libraries (perf-measurement LINK_PUBLIC
			   GaspiCxx
			   pthread
			   rt)

