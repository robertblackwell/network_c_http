#
# This script generates a typed list based on input parameters
#

echo This is mytype_list.sh $(pwd)

base_type=List
base_type_dir=list #"$(tr [A-Z] [a-z] <<< "$base_type")"
base_type_lc=list #"$(tr [A-Z] [a-z] <<< "$base_type")"

# derived type
type=MyType
file_prefix=mytype #"$(tr [A-Z] [a-z] <<< "$type")"
prefix=MT

tmp=$(dirname $(readlink -f ${0}))
project_dir=$(dirname ${tmp})
src_dir=${project_dir}/c_eg

echo This is mytype_list.sh ${project_dir}


#file prefix is lower cased version of type name
template_h=${src_dir}/${base_type_dir}/template.h
generated_h=${src_dir}/${file_prefix}/generated.h
template_c=${src_dir}/${base_type_dir}/template.c
generated_c=${src_dir}/${file_prefix}/generated.c

hand_coded_h=${src_dir}/${file_prefix}/hand_coded.h
outfile_h=${src_dir}/${file_prefix}/${file_prefix}_${base_type_lc}.h
hand_coded_c=${src_dir}/${file_prefix}/hand_coded.c
outfile_c=${src_dir}/${file_prefix}/${file_prefix}_${base_type_lc}.c

final_h=${src_dir}/${file_prefix}_${base_type_lc}.h
final_c=${src_dir}/${file_prefix}_${base_type_lc}.c

if [[ $1 == "clean" ]]; then
  rm -v $generated_h $generated_c $outfile_h $outfile_c
  exit
fi

echo "Generating Typed list for Type: ${type} with prefix ${prefix} "
echo "\tgenerating ${template_h} to ${generated_h}"
echo "\tgenerating ${template_c} to ${generated_c}"

cmd="cat ${template_file} | sed 's/__TYPE__/${type}/g' | sed 's/__PREFIX__/${prefix}/g' > ${generated_h}"

cat ${template_h} | sed "s/__TYPE__/${type}/g" | sed "s/__PREFIX__/${prefix}/g" > ${generated_h}
cat ${template_c} | sed "s/__TYPE__/${type}/g" | sed "s/__PREFIX__/${prefix}/g" > ${generated_c}

# merges together the generated file and hand coded part of the final file
function replace_include() {
  while read  line || [ -n "$line" ]
  do
    if [[ $line == *"__LIST_INCLUDE_H__"* ]]; then
      echo ////////////////////////////////////////////////////////////////////////////////////////////////////////
      echo ///
      echo ///
      echo ///  WARNING The content between these block comments is generated code and will be over written at the next build
      echo ///
      echo ///
      echo ////////////////////////////////////////////////////////////////////////////////////////////////////////
      cat ${2}
      echo ////////////////////////////////////////////////////////////////////////////////////////////////////////
      echo ///
      echo ///
      echo ///  WARNING after this the code is not generated - it comes from the relevant hand_code.h/.c file
      echo ///
      echo ///
      echo ////////////////////////////////////////////////////////////////////////////////////////////////////////
    else
      echo $line
    fi
  done < "${1}"
}

replace_include ${hand_coded_h} ${generated_h}  > ${outfile_h}
cp ${outfile_h} ${final_h}
replace_include ${hand_coded_c} ${generated_c}  > ${outfile_c}
cp ${outfile_c} ${final_c}
