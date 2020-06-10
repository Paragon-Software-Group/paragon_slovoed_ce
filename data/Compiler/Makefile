# Builds all the projects in the solution...
.PHONY: all_projects
all_projects: Compiler 

# Builds project 'Compiler'...
.PHONY: Compiler
Compiler:
	$(MAKE) -C Compiler -f Compiler.makefile

# Cleans all projects...
.PHONY: clean
clean:
	$(MAKE) -C Compiler -f Compiler.makefile clean

