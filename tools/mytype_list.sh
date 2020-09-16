#
# This script generates mytype_list.h and mytype_list.c
#
tmp=$(dirname $(readlink -f ${0}))
project_dir=$(dirname ${tmp})
src_dir=${project_dir}/c_eg
#echo $tmp
#echo project_dir ${project_dir}
#echo src dir ${src_dir}

type=MyType
#file prefix is lower cased version of type name
file_prefix="$(tr [A-Z] [a-z] <<< "$type")"
prefix=MT
input_file_h=${src_dir}/__list_template.h
output_file_h=${src_dir}/${file_prefix}_list_inc.h
input_file_c=${src_dir}/__list_template.c
output_file_c=${src_dir}/${file_prefix}_list_inc.c

infile_h=${src_dir}/${file_prefix}_list_in.h
outfile_h=${src_dir}/${file_prefix}_list.h
infile_c=${src_dir}/${file_prefix}_list_in.c
outfile_c=${src_dir}/${file_prefix}_list.c

echo "Generating Typed list for Type: ${type} with prefix ${prefix} "
echo "\tgenerating ${input_file_h} to ${output_file_h}"
echo "\tgenerating ${input_file_c} to ${output_file_c}"

cmd="cat ${input_file} | sed 's/__TYPE__/${type}/g' | sed 's/__PREFIX__/${prefix}/g' > ${output_file}"

cat ${input_file_h} | sed "s/__TYPE__/${type}/g" | sed "s/__PREFIX__/${prefix}/g" > ${output_file_h}
cat ${input_file_c} | sed "s/__TYPE__/${type}/g" | sed "s/__PREFIX__/${prefix}/g" > ${output_file_c}


while read  line || [ -n "$line" ]
do
  if [[ $line == *"__LIST_INCLUDE_H__"* ]]; then
    echo "Found it"
    echo /// >> ${outfile_h}
    echo /// The remainder of this file is generated code and will be over written at the next build >> ${outfile_h}
    echo /// >> ${outfile_h}
    cat ${output_file_h} >> ${outfile_h}
  else
    echo $line >> ${outfile_h}
  fi
done < "${infile_h}"
