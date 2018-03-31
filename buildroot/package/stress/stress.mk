#############################################################
#
# http://weather.ou.edu/~apw/projects/stress/
#
#############################################################

STRESS_VERSION = 1.0.4
STRESS_SITE    = http://weather.ou.edu/~apw/projects/stress
STRESS_SOURCE  = stress-$(STRESS_VERSION).tar.gz
STRESS_AUTOCONF = YES
	
$(eval $(call AUTOTARGETS,package,stress))
