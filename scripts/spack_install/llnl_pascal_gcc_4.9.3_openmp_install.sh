#
# run at root of ascent repo
#
date
# run spack install, this will install ascent@develop
export BASE_DIR=/usr/gapps/conduit/software/ascent
export ASCENT_VERSION=0.5.2-pre
export DEST_DIR=${BASE_DIR}/${ASCENT_VERSION}/toss_3_x86_64_ib/openmp/gnu
mkdir -p $DEST_DIR
python scripts/uberenv/uberenv.py --spec="%gcc" \
       --install \
       --spack-config-dir="scripts/uberenv/spack_configs/llnl/pascal_openmp/" \
       --prefix=${DEST_DIR}

# gen env helper script
rm public_env.sh
python scripts/spack_install/gen_public_install_env_script.py ${DEST_DIR} gcc/4.9.3
chmod a+x public_env.sh
cp public_env.sh ${BASE_DIR}/${ASCENT_VERSION}/toss_3_x86_64_ib/ascent_toss_3_x86_64_ib_setup_env_gcc_openmp.sh
# change perms to world readable
chmod a+rX -R /usr/gapps/conduit/software/ascent
date

