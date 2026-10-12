ACTIONS = pdf bom ibom gbr step

.PHONY: $(ACTIONS) default help all clean

default: help
	@echo
	@echo "Target required!" && false

help:
	@echo "Usage: make TARGET..."
	@echo
	@echo "Available targets:"
	@echo "  all   Run all commands ($(ACTIONS))."
	@echo "  pdf   Generate PDF schematic."
	@echo "  bom   Generate BOM."
	@echo "  ibom  Generate interactive BOM."
	@echo "  gbr   Generate Gerber files."
	@echo "  step  Generate PCB STEP file."
	@echo "  clean Delete outputs."
	@echo "  help  Show this help message."

all: $(ACTIONS)

KICAD_IBOM_SCRIPT_PATH ?= $(HOME)/.local/share/kicad/7.0/3rdparty/plugins/org_openscopeproject_InteractiveHtmlBom/generate_interactive_bom.py
STEM = ./instrumentation-v3
PCB = $(STEM).kicad_pcb

pdf:
	kicad-cli sch export pdf --no-background-color "$(STEM).kicad_sch" -o "$(STEM).pdf"

BOM_XML = $(STEM).xml
BOM = $(STEM).csv
bom:
	kicad-cli sch export python-bom "$(STEM).kicad_sch" -o "$(BOM_XML)"
	python3 /usr/share/kicad/plugins/bom_csv_grouped_by_value_with_fp.py \
		"$(BOM_XML)" "$(BOM)"
	@echo "Wrote BOM to $(BOM)"

IBOM_DIR = ibom/
ibom:
	python3 "$(KICAD_IBOM_SCRIPT_PATH)" \
		--no-browser --dnp-field kicad_dnp --dest-dir $(IBOM_DIR) \
		"$(PCB)"
	@echo "Wrote interactive BOM to $(IBOM_DIR)"

GBR_DIR = gbr/
gbr:
	mkdir -p "$(GBR_DIR)"
	kicad-cli pcb export gerbers \
		--layers "F.Cu,In1.Cu,In2.Cu,B.Cu,F.Paste,B.Paste,F.Silkscreen,B.Silkscreen,F.Mask,B.Mask,Edge.Cuts" \
		--subtract-soldermask -o "$(GBR_DIR)" "$(PCB)"
	kicad-cli pcb export drill --excellon-separate-th \
		-o "$(GBR_DIR)" "$(PCB)"
	@echo "Wrote fabrication files to $(GBR_DIR)"

STEP = $(STEM).step
step:
	kicad-cli pcb export step --subst-models -o "$(STEP)" "$(PCB)"
	@echo "Wrote STEP model to $(STEP)"

clean:
	rm -rf $(IBOM_DIR) $(GBR_DIR) $(addprefix $(STEM)., pdf step csv xml)
