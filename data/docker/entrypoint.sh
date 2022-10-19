#!/bin/bash
#
# Starts conda env
#

# Conda environment installs/updates
# @see https://github.com/ContinuumIO/docker-images/issues/89#issuecomment-467287039
ENV_NAME="ldm"
ENV_FILE="/home/environment.yaml"
ENV_UPDATED=0
ENV_MODIFIED=$(date -r $ENV_FILE "+%s")
ENV_MODIFED_FILE="/home/.env_updated"
if [[ -f $ENV_MODIFED_FILE ]]; then ENV_MODIFIED_CACHED=$(<${ENV_MODIFED_FILE}); else ENV_MODIFIED_CACHED=0; fi

# Check for updates
conda update -n base -c defaults conda

# Create/update conda env if needed
if ! conda env list | grep ".*${ENV_NAME}.*" >/dev/null 2>&1; then
    echo "Could not find conda env: ${ENV_NAME} ... creating ..."
    conda env create -f $ENV_FILE
    echo "source activate ${ENV_NAME}" > /root/.bashrc
    ENV_UPDATED=1
elif [[ ! -z $CONDA_FORCE_UPDATE && $CONDA_FORCE_UPDATE == "true" ]] || (( $ENV_MODIFIED > $ENV_MODIFIED_CACHED )); then
    echo "Updating conda env: ${ENV_NAME} ..."
    conda env update --file $ENV_FILE --prune
    ENV_UPDATED=1
fi

# Clear artifacts from conda after create/update
# @see https://docs.conda.io/projects/conda/en/latest/commands/clean.html
if (( $ENV_UPDATED > 0 )); then
    conda clean --all
    echo -n $ENV_MODIFIED > $ENV_MODIFED_FILE
fi

# activate conda env
. /opt/conda/etc/profile.d/conda.sh
conda activate $ENV_NAME
conda info | grep active