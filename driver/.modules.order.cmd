cmd_/home/hop/project1/driver/modules.order := {   echo /home/hop/project1/driver/student_driver.ko; :; } | awk '!x[$$0]++' - > /home/hop/project1/driver/modules.order
