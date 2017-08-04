#add executable called "helloDemo" that is built from the source files
# "demo.cxx" and "demo_b.cxx". The extensions are automatically found.
add_executable (1D-wave 
                  1D-wave.cpp )


# Link the executable to the Hello library. Since the Hello library has
# public include directories we will use those link directories when building
# helloDemo

if (GPI2-eth)
  target_link_libraries (1D-wave LINK_PUBLIC 
			   GaspiCxx                           
			   ${GPI2_LIBRARIES} 
			   pthread
			   rt)
else (GPI2-eth)
  target_link_libraries (1D-wave LINK_PUBLIC 
			   GaspiCxx
			   ${GPI2_LIBRARIES} 
			   pthread 
			   ibverbs)
endif (GPI2-eth)
