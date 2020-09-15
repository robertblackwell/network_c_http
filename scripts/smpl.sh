# Install all depepdencies

basedir=$(dirname "$0")
source ${basedir}/common_helpers.sh
source ${basedir}/smpl_install.sh

smpl_clean() {
	echo clean $1
}
smpl_installx (){
	echo install $1
}
smpl_install_only() {
	echo install_only $1
}


if [ "$1" == "help" ] || [ "$1" == "--help" ] || [ "$1" == "-h" ]
then
	smpl_help;

	echo Install all dependencies boost, openssl, catch2, nlohmann_json
	echo Usage:
	echo 	install_dependencies.sh [arg]
	echo
	echo	args is either
	echo		help 	Print this help message
	echo		install After build copy include and libs to final destination
	echo
	echo 	The required packages are downloaded into a temp dir inside the scripts dir
	echo	If required the package is built and the headers and libs copied either
	echo 	to a temporary "stage" directory or to the final location		
	exit 0
elif [ "$1" == "clean" ]
then
	shift
	smpl_clean $@
elif [ "$1" == "install" ]
then
	shift 
	smpl_install "install" $@
elif [ "$1" == "install_only" ]
then
	shift 
	smpl_install_only $@
else
	echo ERROR - $1 is unknown cmd
	exit 1
fi

