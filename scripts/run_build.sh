clear
RESULT=$(ninja -C build/release | grep -C 10 error )

if [ "${RESULT}" ]; then
    echo "${RESULT}"
else
    echo "No errors"
fi

echo -e "\n\n\n"
