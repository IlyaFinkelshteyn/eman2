def getJobType() {
    def causes = "${currentBuild.rawBuild.getCauses()}"
    def job_type = "UNKNOWN"
    
    if(causes ==~ /.*TimerTrigger.*/)    { job_type = "cron" }
    if(causes ==~ /.*GitHubPushCause.*/) { job_type = "push" }
    if(causes ==~ /.*UserIdCause.*/)     { job_type = "manual" }
    if(causes ==~ /.*ReplayCause.*/)     { job_type = "manual" }
    
    return job_type
}

def notifyGitHub(status) {
    if(status == 'PENDING') { message = 'Building...' }
    if(status == 'SUCCESS') { message = 'Build succeeded!' }
    if(status == 'FAILURE') { message = 'Build failed!' }
    if(status == 'ERROR')   { message = 'Build aborted!' }
    step([$class: 'GitHubCommitStatusSetter', contextSource: [$class: 'ManuallyEnteredCommitContextSource', context: "JenkinsCI/${JOB_NAME}"], statusResultSource: [$class: 'ConditionalStatusResultSource', results: [[$class: 'AnyBuildResult', message: message, state: status]]]])
}

pipeline {
  agent {
    node { label 'jenkins-slave-1' }
  }
  
  triggers {
    cron('0-59/1 * * * *')
  }
  
  environment {
    SKIP_UPLOAD = '1'
    JOB_TYPE = getJobType()
  }
  
  stages {
    // Stages triggered by GitHub pushes
    stage('pending') {
      when {
        expression { JOB_TYPE == "push" }
      }
      
      steps {
        notifyGitHub('PENDING')
      }
    }
    
    stage('build') {
      when {
        expression { JOB_TYPE == "push" }
      }
      
      parallel {
        stage('recipe') {
          steps {
            echo 'bash ci_support/build_recipe.sh'
          }
        }
        
        stage('no_recipe') {
          steps {
            echo 'source $(conda info --root)/bin/activate eman-env && bash ci_support/build_no_recipe.sh'
          }
        }
      }
    }
    
    stage('notify') {
      when {
        expression { JOB_TYPE == "push" }
      }
      
      steps {
        echo 'Setting GitHub status...'
      }
      
      post {
        success {
          notifyGitHub('SUCCESS')
        }
        
        failure {
          notifyGitHub('FAILURE')
        }
        
        aborted {
          notifyGitHub('ERROR')
        }
        
        always {
          emailext(recipientProviders: [[$class: 'DevelopersRecipientProvider']],  
                  subject: '[JenkinsCI/$PROJECT_NAME] Build # $BUILD_NUMBER - $BUILD_STATUS!', 
                  body: '''${SCRIPT, template="groovy-text.template"}''')
        }
      }
    }
    
    // Stages triggered by cron
    stage('build-scripts-checkout') {
      when {
        expression { JOB_TYPE == "cron" }
      }
      
      steps {
        echo 'cd ${HOME}/workspace/build-scripts-cron/ && git checkout jenkins && git pull --rebase'
      }
    }
    
    stage('centos6') {
      when {
        expression { JOB_TYPE == "cron" }
        expression { SLAVE_OS == "linux" }
      }
      
      steps {
        echo 'bash ${HOME}/workspace/build-scripts-cron/cronjob.sh centos6'
      }
    }
    
    stage('centos7') {
      when {
        expression { JOB_TYPE == "cron" }
        expression { SLAVE_OS == "linux" }
      }
      
      steps {
        echo 'bash ${HOME}/workspace/build-scripts-cron/cronjob.sh centos7'
      }
    }
    
    stage('mac') {
      when {
        expression { JOB_TYPE == "cron" }
        expression { SLAVE_OS == "osx" }
      }
      
      steps {
        echo 'bash ${HOME}/workspace/build-scripts-cron/cronjob.sh mac'
      }
    }
    
    stage('build-scripts-reset') {
      when {
        expression { JOB_TYPE == "cron" }
      }
      
      steps {
        echo 'cd ${HOME}/workspace/build-scripts-cron/ && git checkout master'
      }
    }
    
    stage('notify-cron') {
      when {
        expression { JOB_TYPE == "cron" }
      }
      
      steps {
        emailext(to: 'shadowwalkersb@gmail.com',  
                          subject: '[JenkinsCI/$PROJECT_NAME/cron] Build # $BUILD_NUMBER - $BUILD_STATUS!', 
                          body: '''${SCRIPT, template="groovy-text.template"}''')
      }
    }
  }
}
