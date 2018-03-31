#! /bin/bash

m_name=$1  
if [ ! -n "$m_name" ]; then  
  echo -e "Usage:$0 [mode_name] \n"
  exit 0  
else  
  echo "Creat a new module..."  
fi 

cp -r ./mode/example/ ./mode/$m_name
#cd mode/$m_name
#rename test ${m_name} *
rename "s/example/${m_name}/" mode/$m_name/*

cd mode/$m_name
sed -i "s/example/${m_name}/g" *
echo "at ./mode/$m_name"  