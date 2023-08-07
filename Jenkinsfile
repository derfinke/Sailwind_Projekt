pipeline {
    agent rockpi

    stages
    {

        stage("build")
        {
            steps {
                echo "building debug"
                echo "building release"
            }
        }
        stage("code-check")
        {
            steps {
                echo "cppcheck"
            }
        }
        stage("doxygen")
        {
            steps {
                echo BRANCH_NAME
                echo "creating doxygen documentation"
            }
        }
    }
}