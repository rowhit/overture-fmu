/*
 * fmi2Functions.c
 *
 *  Created on: May 22, 2015
 *      Author: kel
 */

#include "fmi2Functions.h"

#include "jvmHelper.h"
#include <string.h>


#define FMI2JAVA_CLASS "Fmi2Java"
//int main()
//{
//	// code
//
//	printf("hellow world");
//	return 0; // Zero indicates success, while any
//	// Non-Zero value indicates a failure/error
//}

static JavaVM *jvm;
static JNIEnv *env;

static jlongArray getVrArray(const fmi2ValueReference vr[], size_t nvr)
{
	jlongArray result;
	result = (*env)->NewLongArray(env, nvr);
	if (result == NULL)
	{
		return NULL; /* out of memory error thrown */
	}
	int i;
	// fill a temp structure to use to populate the java int array
	jlong fill[nvr];
	for (i = 0; i < nvr; i++)
	{
		fill[i] = vr[i]; // put whatever logic you want to populate the values here.
	}
	// move from the temp structure to the java structure
	(*env)->SetLongArrayRegion(env, result, 0, nvr, fill);
	return result;
}


char* concat(char *s1, char *s2)
{
    char *result = malloc(strlen(s1)+strlen(s2)+1);//+1 for the zero-terminator
    //in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

// ---------------------------------------------------------------------------
// FMI functions
// ---------------------------------------------------------------------------
fmi2Component fmi2Instantiate(fmi2String instanceName, fmi2Type fmuType, fmi2String fmuGUID,
		fmi2String fmuResourceLocation, const fmi2CallbackFunctions *functions, fmi2Boolean visible,
		fmi2Boolean loggingOn)
{
	printf("instantiate\n");

	FILE * fp;

	size_t len = 0;
	ssize_t read;

	const char* configPath = "/config.ini";
	char *result = malloc(strlen(fmuResourceLocation) + strlen(configPath) + 1);

	strcpy(result, fmuResourceLocation);
	strcat(result, configPath);

	fp = fopen(result, "r");
	free(result);
	if (fp == NULL)
	{
		printf("Faild to open file\n");
		return NULL;
	}

	char * line = NULL;
	char * jvmLibPath = NULL;
	char * jvmOption1 = NULL;
	int i = 0;


	while ((read = getdelim(&line, &len, '\n',fp)) != -1)
	{
//		printf("Retrieved line of length %zu :\n", read);
//		printf("%s\n", line);



		if (i == 0)
		{
			jvmLibPath = malloc(strlen (line));
			stpncpy(jvmLibPath, line, strlen (line)-1);
			//strcpy(jvmLibPath,line);
			jvmLibPath[strlen (line)-1]=0;
			printf("---- stpncpy line of length '%s' :\n", jvmLibPath);
		} else if (i == 1)
		{
			jvmOption1 = malloc(strlen (line)-1);

			stpncpy(jvmOption1, line, strlen (line));
			jvmOption1[strlen (line)]=0;
			printf("---- stpncpy line of length '%s' :\n", jvmOption1);

			jvmOption1 =concat(fmuResourceLocation,jvmOption1);
			printf("---- stpncpy CP '%s' :\n", jvmOption1);

		}
		i++;
	}

	fclose(fp);
	if (line)
		free(line);

//	printf("JVM lib path: %s\n", jvmLibPath);
//	printf("JVM lib opt1: %s\n", jvmOption1);

	env = create_vm(&jvm, jvmLibPath, jvmOption1);

	if (env == NULL)
		printf("unable to create VM\n");

	printf("final test\n");
	callJavaMethodVoid(env, FMI2JAVA_CLASS, "instantiate", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Z)V",
			(*env)->NewStringUTF(env, fmuGUID), (*env)->NewStringUTF(env, instanceName),(*env)->NewStringUTF(env, fmuResourceLocation), 0);
//	printf("%d ", status);

	return (void*)1234;
}

fmi2Status fmi2SetupExperiment(fmi2Component c, fmi2Boolean toleranceDefined, fmi2Real tolerance, fmi2Real startTime,
		fmi2Boolean stopTimeDefined, fmi2Real stopTime)
{

	return fmi2OK;
}

fmi2Status fmi2EnterInitializationMode(fmi2Component c)
{
	callJavaMethodByte(env, FMI2JAVA_CLASS, "enterInitializationMode", "()B");
	return fmi2OK;
}

fmi2Status fmi2ExitInitializationMode(fmi2Component c)
{
	callJavaMethodByte(env, FMI2JAVA_CLASS, "exitInitializationMode", "()B");
	return fmi2OK;
}

fmi2Status fmi2Terminate(fmi2Component c)
{
	callJavaMethodByte(env, FMI2JAVA_CLASS, "terminate", "()B");
	return fmi2OK;
}

fmi2Status fmi2Reset(fmi2Component c)
{
	callJavaMethodByte(env, FMI2JAVA_CLASS, "reset", "()B");
	return fmi2Error;
}

void fmi2FreeInstance(fmi2Component c)
{
	callJavaMethodByte(env, FMI2JAVA_CLASS, "freeInstance", "()B");

	//TODO free stuff
}

// ---------------------------------------------------------------------------
// FMI functions: class methods not depending of a specific model instance
// ---------------------------------------------------------------------------

const char* fmi2GetVersion()
{
	return fmi2Version;
}

const char* fmi2GetTypesPlatform()
{
	return fmi2TypesPlatform;
}

// ---------------------------------------------------------------------------
// FMI functions: logging control, setters and getters for Real, Integer,
// Boolean, String
// ---------------------------------------------------------------------------

fmi2Status fmi2SetDebugLogging(fmi2Component c, fmi2Boolean loggingOn, size_t nCategories,
		const fmi2String categories[])
{
	return fmi2OK;
}

fmi2Status fmi2GetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Real value[])
{
	int i = 0;

	jdoubleArray valArr;
	valArr = (*env)->NewDoubleArray(env, nvr);
	if (valArr == NULL)
	{
		return fmi2Error; /* out of memory error thrown */
	}

	callJavaMethodByte(env, FMI2JAVA_CLASS, "getReal", "([J[D)B", getVrArray(vr, nvr), valArr);

	jdouble *vbody = (*env)->GetDoubleArrayElements(env, valArr, 0);
	for (i = 0; i < nvr; i++)
	{
		value[i] = vbody[i];
	}

	(*env)->ReleaseDoubleArrayElements(env, valArr, vbody, 0);

	return fmi2OK;
}

fmi2Status fmi2GetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Integer value[])
{
	return fmi2Error;
}

fmi2Status fmi2GetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2Boolean value[])
{
	int i = 0;

		jbooleanArray valArr;
		valArr = (*env)->NewBooleanArray(env, nvr);
		if (valArr == NULL)
		{
			return fmi2Error; /* out of memory error thrown */
		}

		callJavaMethodByte(env, FMI2JAVA_CLASS, "getBoolean", "([J[Z)B", getVrArray(vr, nvr), valArr);

		jboolean *vbody = (*env)->GetBooleanArrayElements(env, valArr, 0);
		for (i = 0; i < nvr; i++)
		{
			value[i] = vbody[i];
		}

		(*env)->ReleaseBooleanArrayElements(env, valArr, vbody, 0);

		return fmi2OK;
}

fmi2Status fmi2GetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, fmi2String value[])
{
	return fmi2Error;
}



fmi2Status fmi2SetReal(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Real value[])
{
	int i = 0;

	jdoubleArray valArr;
	valArr = (*env)->NewDoubleArray(env, nvr);
	if (valArr == NULL)
	{
		return fmi2Error; /* out of memory error thrown */
	}
	jdouble fill2[nvr];
	for (i = 0; i < nvr; i++)
	{
		fill2[i] = value[i]; // put whatever logic you want to populate the values here.
	}
	// move from the temp structure to the java structure
	(*env)->SetDoubleArrayRegion(env, valArr, 0, nvr, fill2);

	callJavaMethodByte(env, FMI2JAVA_CLASS, "setReal", "([J[D)B", getVrArray(vr, nvr), valArr);

	return fmi2OK;
}

fmi2Status fmi2SetInteger(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Integer value[])
{
	return fmi2Error;
}

fmi2Status fmi2SetBoolean(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2Boolean value[])
{
	int i = 0;

		jbooleanArray valArr;
		valArr = (*env)->NewBooleanArray(env, nvr);
		if (valArr == NULL)
		{
			return fmi2Error; /* out of memory error thrown */
		}
		jboolean fill2[nvr];
		for (i = 0; i < nvr; i++)
		{
			fill2[i] = value[i]; // put whatever logic you want to populate the values here.
		}
		// move from the temp structure to the java structure
		(*env)->SetBooleanArrayRegion(env, valArr, 0, nvr, fill2);

		callJavaMethodByte(env, FMI2JAVA_CLASS, "setBoolean", "([J[Z)B", getVrArray(vr, nvr), valArr);

		return fmi2OK;
}

fmi2Status fmi2SetString(fmi2Component c, const fmi2ValueReference vr[], size_t nvr, const fmi2String value[])
{
	return fmi2Error;
}

fmi2Status fmi2GetFMUstate(fmi2Component c, fmi2FMUstate* FMUstate)
{
	return fmi2Error;
}
fmi2Status fmi2SetFMUstate(fmi2Component c, fmi2FMUstate FMUstate)
{
	return fmi2Error;
}
fmi2Status fmi2FreeFMUstate(fmi2Component c, fmi2FMUstate* FMUstate)
{
	return fmi2Error;
}
fmi2Status fmi2SerializedFMUstateSize(fmi2Component c, fmi2FMUstate FMUstate, size_t *size)
{
	return fmi2Error;
}
fmi2Status fmi2SerializeFMUstate(fmi2Component c, fmi2FMUstate FMUstate, fmi2Byte serializedState[], size_t size)
{
	return fmi2Error;
}
fmi2Status fmi2DeSerializeFMUstate(fmi2Component c, const fmi2Byte serializedState[], size_t size,
		fmi2FMUstate* FMUstate)
{
	return fmi2Error;
}

fmi2Status fmi2GetDirectionalDerivative(fmi2Component c, const fmi2ValueReference vUnknown_ref[], size_t nUnknown,
		const fmi2ValueReference vKnown_ref[], size_t nKnown, const fmi2Real dvKnown[], fmi2Real dvUnknown[])
{
	return fmi2Error;
}

// ---------------------------------------------------------------------------
// Functions for FMI for Co-Simulation
// ---------------------------------------------------------------------------
#ifdef FMI_COSIMULATION
/* Simulating the slave */
fmi2Status fmi2SetRealInputDerivatives(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
		const fmi2Integer order[], const fmi2Real value[])
{
	return fmi2Error;
}

fmi2Status fmi2GetRealOutputDerivatives(fmi2Component c, const fmi2ValueReference vr[], size_t nvr,
		const fmi2Integer order[], fmi2Real value[])
{
	return fmi2Error;
}

fmi2Status fmi2CancelStep(fmi2Component c)
{
	return fmi2Error;
}

fmi2Status fmi2DoStep(fmi2Component c, fmi2Real currentCommunicationPoint, fmi2Real communicationStepSize,
		fmi2Boolean noSetFMUStatePriorToCurrentPoint)
{
	callJavaMethodVoid(env, FMI2JAVA_CLASS, "doStep", "(DD)V",currentCommunicationPoint,communicationStepSize);
	return fmi2OK;
}

fmi2Status fmi2GetStatus(fmi2Component c, const fmi2StatusKind s, fmi2Status *value)
{
	return fmi2Error;
}

fmi2Status fmi2GetRealStatus(fmi2Component c, const fmi2StatusKind s, fmi2Real *value)
{
	return fmi2Error;
}

fmi2Status fmi2GetIntegerStatus(fmi2Component c, const fmi2StatusKind s, fmi2Integer *value)
{
	return fmi2Error;
}

fmi2Status fmi2GetBooleanStatus(fmi2Component c, const fmi2StatusKind s, fmi2Boolean *value)
{
	return fmi2Error;
}

fmi2Status fmi2GetStringStatus(fmi2Component c, const fmi2StatusKind s, fmi2String *value)
{
	return fmi2Error;
}

// ---------------------------------------------------------------------------
// Functions for FMI2 for Model Exchange
// ---------------------------------------------------------------------------
#else
/* Enter and exit the different modes */
fmi2Status fmi2EnterEventMode(fmi2Component c)
{
	return fmi2Error;
}

fmi2Status fmi2NewDiscreteStates(fmi2Component c, fmi2EventInfo *eventInfo)
{
	return fmi2Error;
}

fmi2Status fmi2EnterContinuousTimeMode(fmi2Component c)
{
	return fmi2Error;
}

fmi2Status fmi2CompletedIntegratorStep(fmi2Component c, fmi2Boolean noSetFMUStatePriorToCurrentPoint,
		fmi2Boolean *enterEventMode, fmi2Boolean *terminateSimulation)
{
	return fmi2Error;
}

/* Providing independent variables and re-initialization of caching */
fmi2Status fmi2SetTime(fmi2Component c, fmi2Real time)
{
	return fmi2Error;
}

fmi2Status fmi2SetContinuousStates(fmi2Component c, const fmi2Real x[], size_t nx)
{
	return fmi2Error;
}

/* Evaluation of the model equations */
fmi2Status fmi2GetDerivatives(fmi2Component c, fmi2Real derivatives[], size_t nx)
{
	return fmi2Error;
}

fmi2Status fmi2GetEventIndicators(fmi2Component c, fmi2Real eventIndicators[], size_t ni)
{
	return fmi2Error;
}

fmi2Status fmi2GetContinuousStates(fmi2Component c, fmi2Real states[], size_t nx)
{
	return fmi2Error;
}

fmi2Status fmi2GetNominalsOfContinuousStates(fmi2Component c, fmi2Real x_nominal[], size_t nx)
{
	return fmi2Error;
}
#endif // Model Exchange
