#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

//max. number of patient threads:
#define MAX_PATIENTS 1000

//There is a code that adds randomness to the patients arrivel times in line 170.
//You can activate it if you want but it significantly lowers the wait operations so it is turned off.

int PATIENT_NUMBER = 1000; //Change this value for different simulation outputs.

int REGISTRATION_SIZE = 10;

int RESTROOM_SIZE = 10;

int CAFE_NUMBER = 10;

int GP_NUMBER = 10;

int PHARMACY_NUMBER = 10;

int BLOOD_LAB_NUMBER = 10;

int OR_NUMBER = 10;
int SURGEON_NUMBER = 30;
int NURSE_NUMBER = 30;

int SURGEON_LIMIT = 5;
int NURSE_LIMIT = 5;

int HOSPITAL_WALLET = 0;

int ARRIVAL_TIME = 100;
int WAIT_TIME = 100;
int REGISTRATION_TIME = 100;
int GP_TIME = 200;
int PHARMACY_TIME = 100;
int BLOOD_LAB_TIME = 200;
int SURGERY_TIME = 500;
int CAFE_TIME = 100;
int RESTROOM_TIME = 100;

int REGISTRATION_COST = 100;
int PHARMACY_COST = 200; // Calculated randomly between 1 and given value.
int BLOOD_LAB_COST = 200;
int SURGERY_OR_COST = 200;
int SURGERY_SURGEON_COST = 100;
int SURGERY_NURSE_COST = 50;
int CAFE_COST = 200; // Calculated randomly between 1 and given value.

int HUNGER_INCREASE_RATE = 10;
int RESTROOM_INCREASE_RATE = 10;

int Hunger_Meter;   // Initialized between 1 and 100 at creation.
int Restroom_Meter; // Initialized between 1 and 100 at creation.

void *patient(void *num);

//define the semaphores.

// Initialize with  REGISTRATION_SIZE
sem_t registration_office;

// Initialize with RESTROOM_SIZE
sem_t restroom;

// Initialize with CAFE_NUMBER
sem_t cafe;

// Initialize with GP_NUMBER
sem_t gp_doctors;

// Initialize with PHARMACY_NUMBER
sem_t pharmacy;

// Initialize with BLOOD_LAB_NUMBER
sem_t blood_lab;

sem_t operation_room;

// Initialize with SURGEON_NUMBER
//sem_t surgery;

pthread_mutex_t surgery;

int allDone = 0;

//Struct for keeping hunger and restroom meters.
struct humans
{

    int Hunger;
    int Restroom;
    int patient_id;
};

struct humans arr_h[MAX_PATIENTS];
int rnd_hunger_number;
int rnd_restroom_number;

//function to create structs.
void create_struct()
{

    for (int i = 0; i < PATIENT_NUMBER; i++)
    {
        rnd_hunger_number = rand() % 100 + 1;   // random number for hunger_meter
        rnd_restroom_number = rand() % 100 + 1; // random number for restroom_meter

        arr_h[i].patient_id = i;
        arr_h[i].Hunger = rnd_hunger_number;
        arr_h[i].Restroom = rnd_restroom_number;
    }
}

//function for random usleeps.
void rnd_sleep(int milisecs)
{
    int wait = (rand() % milisecs) + 1;
    usleep(wait * 1000);
}

int main(int argc, char argv[])
{

    srand(time(NULL));
    int arr_time;

    printf("The day has started.\n");

    pthread_t tid[PATIENT_NUMBER]; // Thread ID's for patients.
    int Number[PATIENT_NUMBER];
    for (int i = 0; i < PATIENT_NUMBER; i++) //Assigning patient Thread ID's.
    {
        Number[i] = i;
        arr_h[i].patient_id = i;
    }

    //Initializing the semaphores.
    sem_init(&registration_office, 0, REGISTRATION_SIZE);
    sem_init(&restroom, 0, RESTROOM_SIZE);
    sem_init(&cafe, 0, CAFE_NUMBER);
    sem_init(&gp_doctors, 0, GP_NUMBER);
    sem_init(&pharmacy, 0, PHARMACY_NUMBER);
    sem_init(&blood_lab, 0, BLOOD_LAB_NUMBER);
    sem_init(&operation_room, 0, OR_NUMBER);
    //sem_init(&surgery, 0, SURGEON_NUMBER);

    pthread_mutex_init(&surgery, NULL);

    create_struct;

    //Create the patients.
    for (int i = 0; i < PATIENT_NUMBER; i++)
    {
        pthread_create(&tid[i], NULL, &patient, (void *)&Number[i]);

        rnd_hunger_number = rand() % 100 + 1;   // random number for hunger_meter
        rnd_restroom_number = rand() % 100 + 1; // random number for restroom_meter

        //arr_h[i].patient_id = i;
        arr_h[i].Hunger = rnd_hunger_number;
        arr_h[i].Restroom = rnd_restroom_number;

        //TO ADD RANDOMNESS
        //rnd_sleep(ARRIVAL_TIME);
    }

    //join each of the threads to wait for them to finish.
    for (int i = 0; i < PATIENT_NUMBER; i++)
    {
        pthread_join(tid[i], NULL);
    }

    allDone = 1;

    printf("End of the day, everyone is going home. \n");
    printf("HOSPITAL WALLET:[%d] \n", HOSPITAL_WALLET);
    system("read -p 'Press Enter to continue...' var");

    return 0;
}

int rnd_cafe_cost;
int rnd_pharmacy_cost;
int surgery_cost;

//Function to increase the meters.
void *increase_meters(void *number)
{

    int num = *(int *)number;
    //Increase the Hungermeter and Restroom meter between 1-10.
    int rnd_hunger_increase = rand() % HUNGER_INCREASE_RATE + 1;
    int rnd_restroom_increase = rand() % RESTROOM_INCREASE_RATE + 1;
    arr_h[num].Hunger += rnd_hunger_increase;
    arr_h[num].Restroom += rnd_restroom_increase;
}

//Function to check the meters.
void *check_meters(void *number)
{

    int num = *(int *)number;

    // Check hunger meter and restroom meter. Increase them at the end of waiting periods.
    if (arr_h[num].Hunger >= 100)
    {
        //Goes to cafe.
        while (1)
        {
            if (sem_trywait(&cafe) == 0)
            {
                printf("Patient %d is in the Cafe. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
                arr_h[num].Hunger = 0; //Reset hungermeter.
                rnd_sleep(CAFE_TIME);
                rnd_cafe_cost = rand() % CAFE_COST + 1; //RANDOM CAFE COST
                HOSPITAL_WALLET += rnd_cafe_cost;
                sem_post(&cafe); //Leave cafe.
                break;
            }
            else
            {
                printf("Patient %d is waiting for his/her turn at the Cafe. \n", num);
                rnd_sleep(WAIT_TIME);
            }
        }
        printf("Patient %d is leaving Cafe. \n", num);
    }
    else if (arr_h[num].Restroom >= 100)
    {
        //Goes to restroom.
        while (1)
        {
            if (sem_trywait(&restroom) == 0)
            {
                printf("Patient %d is in the Restroom. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
                arr_h[num].Restroom = 0; //Reset restroom meter.
                rnd_sleep(RESTROOM_TIME);

                //Leave restroom.
                sem_post(&restroom);
                break;
            }
            else
            {
                printf("Patient %d is waiting for his/her turn at the Restroom. \n", num);
                rnd_sleep(WAIT_TIME);
            }
        }
        printf("Patient %d is leaving Restroom. \n", num);
    }
    else //During any waiting period, increase hunger and restroom need.
    {
        increase_meters(&num);
    }
}

//Function for day-cycle of a patient.
void *patient(void *number)
{

    int num = *(int *)number;

    //Patient is on his/her way for the hospital.
    printf("Patient %d is leaving for hospital.\n", num);
    rnd_sleep(ARRIVAL_TIME);
    increase_meters(&num);

    //Patient arrived at hospital.
    printf("Patient %d arrived at hospital. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);

    //Patient enters the registration room to register. If it is full, patient will wait for his/her turn.
    while (1)
    {
        if (sem_trywait(&registration_office) == 0)
        {
            printf("Patient %d is registering. \n", num);
            rnd_sleep(REGISTRATION_TIME);
            increase_meters(&num);
            HOSPITAL_WALLET += REGISTRATION_COST;
            //Registration completed. Proceed to GP's office.If he/she needs to go to restroom or cafe, he/she goes.
            sem_post(&registration_office);
            break;
        }
        else
        {
            printf("Patient %d is waiting for an available desk at Registration Office.\n", num);
            rnd_sleep(WAIT_TIME);
            increase_meters(&num);
            check_meters(&num);
        }
    }

    printf("Patient %d registered successfully. Leaving for GP's office. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
    check_meters(&num);

    //Patient goes to GP's office.
    while (1)
    {

        if (sem_trywait(&gp_doctors) == 0)
        {
            printf("Patient %d is in the GP's office.  \n", num);
            rnd_sleep(GP_TIME);
            increase_meters(&num);
            //Examination is over. Patient leaves GP's office.
            sem_post(&gp_doctors);
            break;
        }
        else
        {
            printf("Patient %d is waiting for GP's office to be available. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
            rnd_sleep(WAIT_TIME);
            increase_meters(&num);
            check_meters(&num);
        }
    }

    printf("Patient %d is examined and leaving GP's office. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
    check_meters(&num);

    int sayi = rand() % 3 + 1; // [1,3] Generating random nuber 1-3 for 3 different cases after the GP's examination.
    if (sayi == 1)             //1st CASE , patient is predescribed medicine by GP , goes to the pharmacy ,then goes home.
    {
        printf("Patient %d is predescribed medicine by GP and going to the Pharmacy. \n", num);
        rnd_sleep(ARRIVAL_TIME);
        increase_meters(&num);
        printf("Patient %d arrived at Pharmacy. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
        check_meters(&num);

        //Patient is buying medicine. Patient will wait for the turn if there is no available cashiers.
        while (1)
        {
            if (sem_trywait(&pharmacy) == 0)
            {
                rnd_pharmacy_cost = rand() % PHARMACY_COST + 1; //RANDOM PHARMACY COST
                HOSPITAL_WALLET += rnd_pharmacy_cost;
                printf("Patient %d is buying medicine at Pharmacy. Patient %d's Pharmacy payment: [%d] \n", num, num, rnd_pharmacy_cost);
                rnd_sleep(PHARMACY_TIME);
                increase_meters(&num);
                //Patient bought the medicine. Patient will go home (but he/she may go the restroom or cafe if he/she needs.).
                sem_post(&pharmacy);
                break;
            }
            else
            {
                printf("Patient %d is waiting for an available cashier at Pharmacy.[HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
                rnd_sleep(WAIT_TIME);
                increase_meters(&num);
                check_meters(&num);
            }
        }
        check_meters(&num);
        printf("Patient %d is leaving Pharmacy and going home.  \n", num);
    }
    else if (sayi == 2)
    { //2nd CASE. Patient goes to the Blood Lab. After the test results, patient will return to the GP's office.

        printf("Patient %d is leaving GP's office and going to the Blood Lab. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
        rnd_sleep(ARRIVAL_TIME);
        increase_meters;
        check_meters;

        //Patient is giving blood.
        while (1)
        {
            if (sem_trywait(&blood_lab) == 0)
            {
                printf("Patient %d is giving blood at Blood Lab.Blood Lab payment: [%d] \n", num, BLOOD_LAB_COST);
                rnd_sleep(BLOOD_LAB_TIME);
                increase_meters(&num);
                sem_post(&blood_lab);
                HOSPITAL_WALLET += BLOOD_LAB_COST;
                break;
            }
            else
            {
                printf("Patient %d is waiting for an assistant to be available at Blood Lab. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
                rnd_sleep(WAIT_TIME);
                increase_meters(&num);
                check_meters(&num);
            }
        }

        printf("Patient %d is leaving Blood Lab and going to GP's office. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
        check_meters(&num);

        //Patient goes to GP's office.
        while (1)
        {
            if (sem_trywait(&gp_doctors) == 0)
            {
                printf("Patient %d is in the GP's office. \n", num);
                rnd_sleep(GP_TIME);
                increase_meters(&num);
                sem_post(&gp_doctors);
                break;
            }
            else
            {
                printf("Patient %d is waiting for GP to be available. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
                rnd_sleep(WAIT_TIME);
                increase_meters(&num);
                check_meters(&num);
            }
        }

        printf("Patient %d is examined by GP. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
        check_meters(&num);

        sayi = rand() % 2 + 1; // [1,2] random number for 2 cases.
        if (sayi == 1)
        { // 1st CASE. Patient will go to the pharmacy and buy medicine.

            printf("Patient %d is predescribed medicine by GP and going to the Pharmacy  \n", num);

            rnd_sleep(ARRIVAL_TIME);
            increase_meters(&num);
            printf("Patient %d arrived at pharmacy. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
            check_meters(&num);

            //patient is buying medicine.
            while (1)
            {

                if (sem_trywait(&pharmacy) == 0)
                {
                    rnd_pharmacy_cost = rand() % PHARMACY_COST + 1; //RANDOM PHARMACY COST
                    printf("Patient %d is buying medicine at Pharmacy.Pharmacy payment: [%d] \n", num, rnd_pharmacy_cost);
                    rnd_sleep(PHARMACY_TIME);
                    increase_meters(&num);
                    sem_post(&pharmacy);
                    HOSPITAL_WALLET += rnd_pharmacy_cost;
                    break;
                }
                else
                {

                    printf("Patient %d is waiting for available cashier at Pharmacy. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
                    rnd_sleep(WAIT_TIME);
                    increase_meters(&num);
                    check_meters(&num);
                }
            }
            check_meters(&num);
            printf("Patient %d is going home.  \n", num);
        }
        else if (sayi == 2)
        { //Patient goes directly to home without going to the pharmacy.
            check_meters(&num);
            printf("Patient % is going home.  \n", num);
        }
    }

    //SURGERY CASE
    else if (sayi == 3)
    { //3rd Case. Patient will have a surgery.
        //Patient goes to operation room.
        //Random number of nurses and surgeons will be needed. 1 OR is needed.
        int nurses = rand() % NURSE_LIMIT + 1;     //1-5 nurses required.
        int surgeons = rand() % SURGEON_LIMIT + 1; //1-5 surgeons required.
        printf("Patient %d needs surgery, going to Operation Room. Number of needed Surgeons: %d Nurses: %d . \n", num, surgeons, nurses);
        rnd_sleep(ARRIVAL_TIME);
        increase_meters(&num);
        check_meters(&num);

        //sem_trywait(&operation_room)==0

        while (1)
        {
            pthread_mutex_lock(&surgery);
            //Do the surgery if nurses, surgeons and OR are available.
            if (SURGEON_NUMBER >= surgeons && NURSE_NUMBER >= nurses && OR_NUMBER > 0)
            {

                //Organizing nurse,surgeon and Or numbers for surgery.
                NURSE_NUMBER = NURSE_NUMBER - nurses;
                SURGEON_NUMBER = SURGEON_NUMBER - surgeons;
                OR_NUMBER = OR_NUMBER - 1;

                surgery_cost = SURGERY_OR_COST + (surgeons * SURGERY_SURGEON_COST) + (nurses * SURGERY_NURSE_COST);
                HOSPITAL_WALLET += surgery_cost;

                printf("Patient %d is having a surgery. Payment for surgery: [%d]. \n", num, surgery_cost);

                rnd_sleep(SURGERY_TIME);

                // SURGERY_OR_COST + (number of surgeons * SURGERY_SURGEON_COST) +  (number of nurses * SURGERY_NURSE_COST)

                sem_post(&operation_room);
                pthread_mutex_unlock(&surgery);
                break;
            }
            else
            {
                printf("Patient %d is waiting for Surgery. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
                rnd_sleep(WAIT_TIME);
                increase_meters(&num);
                check_meters(&num);
            }
        }

        //Reorganizing nurse,surgeon and Or numbers for surgery.
        NURSE_NUMBER = NURSE_NUMBER + nurses;
        SURGEON_NUMBER = SURGEON_NUMBER + surgeons;
        OR_NUMBER = OR_NUMBER + 1;

        printf("Patient %d is leaving OR and going to GP's office.  \n", num);
        check_meters(&num);

        //Patient will go to GP's office.
        while (1)
        {

            if (sem_trywait(&gp_doctors) == 0)
            {
                printf("Patient %d is in the GP's office.  \n", num);
                rnd_sleep(GP_TIME);
                increase_meters(&num);
                sem_post(&gp_doctors);
                break;
            }
            else
            {
                printf("Patient %d is waiting for GP to be available. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
                rnd_sleep(WAIT_TIME);
                increase_meters(&num);
                check_meters(&num);
            }
        }

        printf("Patient %d is examined by GP. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
        check_meters(&num);

        sayi = rand() % 2 + 1; // [1,2] random number 1-2 for 2 cases.
        if (sayi == 1)
        { // 1st CASE. Patient goes to the pharmacy.

            printf("Patient %d is predescribed medicine by GP.  \n", num);
            printf("Patient %d is going to the Pharmacy.\n", num);
            rnd_sleep(ARRIVAL_TIME);
            increase_meters(&num);
            printf("Patient %d arrived at pharmacy. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
            check_meters(&num);

            while (1)
            {
                if (sem_trywait(&pharmacy) == 0)
                {
                    rnd_pharmacy_cost = rand() % PHARMACY_COST + 1; //RANDOM PHARMACY COST
                    printf("Patient %d is buying medicine at Pharmacy. Pharmacy payment: [%d] \n", num, rnd_pharmacy_cost);
                    rnd_sleep(PHARMACY_TIME);
                    increase_meters(&num);
                    sem_post(&pharmacy);
                    HOSPITAL_WALLET += rnd_pharmacy_cost;
                    break;
                }
                else
                {

                    printf("Patient %d is waiting for available cashier at Pharmacy. [HUNGER: %d]   [RESTROOM: %d] \n", num, arr_h[num].Hunger, arr_h[num].Restroom);
                    rnd_sleep(WAIT_TIME);
                    increase_meters(&num);
                    check_meters(&num);
                }
            }
            check_meters(&num);
            printf("Patient %d is leaving pharmacy and going home. \n", num);
        }
        else if (sayi == 2)
        { //Patient goes directly home.

            printf("Patient %d is going home. \n", num);
        }
    }
}