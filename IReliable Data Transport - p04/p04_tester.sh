#!/bin/bash

GROUP_NUM=1  # TODO: modify this to be your group num

# DO NOT MODIFY ANYTHING BELOW THIS POINT

SENDER_PORT=$(($GROUP_NUM + 8000))
RECEIVER_PORT=$(($GROUP_NUM + 9000))

OUTPUT_DIR=test_output/self-testing

if [ ! -d "$OUTPUT_DIR" ]; then
  mkdir -p $OUTPUT_DIR
fi

configs=(
	'1,0,0.0,5lines.txt,5s'
	'5,40,0.1,5lines.txt,5s'
	'2,10,0.5,5lines.txt,10s'
	'5,40,0.1,1000lines.txt,10s'
	'5,40,0.25,1000lines.txt,20s'
	'2,80,0.1,75lines.txt,10s'
	'2,150,0.1,75lines.txt,10s'
	)

trap 'kill $(jobs -p)' EXIT

TEST_NUM=0
for c in "${configs[@]}"; do
	OLDIFS=$IFS; IFS=','
	set -- $c
	IFS=$OLDIFS

	NUM_ITERS=$1
	DELAY=$2
	LOSS_RATE=$3
	FILE_TO_SEND=$4
	MAX_TIME=$5

	#echo "Sending $FILE_TO_SEND $NUM_ITERS times with delay=$DELAY loss=$LOSS_RATE, max runtime=$MAX_TIME"

	for i in `seq 1 $NUM_ITERS`; do
		TEST_DESCRIPTION="Test $TEST_NUM: Sending $FILE_TO_SEND with delay=$DELAY loss=$LOSS_RATE, max runtime=$MAX_TIME"
		RECEIVED_FILE="$OUTPUT_DIR/test${TEST_NUM}.received.$FILE_TO_SEND"
		NETWORK_LOG="$OUTPUT_DIR/test${TEST_NUM}.mock_network.log"
		RECEIVER_LOG="$OUTPUT_DIR/test${TEST_NUM}.receiver.log"
		SENDER_LOG="$OUTPUT_DIR/test${TEST_NUM}.sender.log"

		echo $TEST_DESCRIPTION | tee -a $OUTPUT_DIR/test_case_descripitions.txt

		python3 mock_network.py -s 127.0.0.1:$SENDER_PORT -d 127.0.0.1:$RECEIVER_PORT --delay=$DELAY --loss=$LOSS_RATE -v &> $NETWORK_LOG &
		NETWORK_PID=$!
		sleep 1

		timeout -k $MAX_TIME $MAX_TIME ./receiver $RECEIVER_PORT $RECEIVED_FILE -v &> $RECEIVER_LOG &
		RECEIVER_PID=$!
		sleep 1

		timeout -k $MAX_TIME $MAX_TIME ./sender 127.0.0.1 $SENDER_PORT $FILE_TO_SEND -v &> $SENDER_LOG &
		SENDER_PID=$!

		wait $RECEIVER_PID
		RECEIVER_EXIT_STATUS=$?
		wait $SENDER_PID
		SENDER_EXIT_STATUS=$?

		# stop the mock network
		kill $NETWORK_PID

		let "TEST_NUM+=1"

		if [[ "${RECEIVER_EXIT_STATUS}" = "124" ]] || [[ "${SENDER_EXIT_STATUS}" = "124" ]]; then
			echo "Test case FAILED (exceeded max runtime)"
			echo "(exceeded max runtime) $TEST_DESCRIPTION" >> $OUTPUT_DIR/failed_tests.txt
			continue
		fi

		if [[ "${RECEIVER_EXIT_STATUS}" != "0" ]] || [[ "${SENDER_EXIT_STATUS}" != "0" ]]; then
			echo "Test case FAILED (exited abnormally)"
			echo "(exited abnormally) $TEST_DESCRIPTION" >> $OUTPUT_DIR/failed_tests.txt
			continue
		fi

		cmp $RECEIVED_FILE $FILE_TO_SEND
		if [[ "$?" != "0" ]]; then
			echo "Test Case FAILED (incorrect result)"
			echo "(incorrect result) $TEST_DESCRIPTION" >> $OUTPUT_DIR/failed_tests.txt
		fi

	done
done
