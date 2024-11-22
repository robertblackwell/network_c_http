prefix=${1}
ucf_prefix="${prefix^}"
echo $prefix   $ucf_prefix
mkdir -p ${prefix}
rm ${prefix}/*

function edit_file () {
    echo "Editing ${1}"
    cp ./template_${1} ./${prefix}/${prefix}_${1}
    sed -i "s/Tmpl/${ucf_prefix}/g" ./${prefix}/${prefix}_${1}
    sed -i "s/tmpl/${prefix}/g" ./${prefix}/${prefix}_${1}

    diff ./${prefix}/${prefix}_${1} ../http_in_c/${prefix}_protocol/${prefix}_${1}
}

edit_file connection.c
edit_file connection.h
edit_file handler.h
edit_file handler.c
edit_file server.h
edit_file server.c
edit_file sync_socket.h
edit_file sync_socket.c