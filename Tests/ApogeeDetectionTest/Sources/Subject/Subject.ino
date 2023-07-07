#include <TaskManager.h>
#include "Trajectory.hpp"


Trajectory trajectory(0.25, 0.75);
float altitudes[] = { -0.333836651, 0.34489738, -1.631194748, -1.495469341, -1.425238607, -1.904218156, -0.472734654, 0.483017532, 2.664780959, 3.89164097, 4.436434255, 5.868830187, 8.871687405, 9.143426712, 9.75723728, 13.17141003, 15.28560791, 17.67697101, 20.47697941, 23.2105347, 26.89917668, 29.49676276, 31.20590404, 35.03531901, 36.74541158, 40.44065816, 42.4940777, 44.13727687, 47.14932382, 51.94399707, 54.41163532, 60.16892599, 65.24314938, 67.91909813, 70.45905428, 75.9504663, 79.93556366, 84.47065083, 88.18206184, 92.99335065, 96.43698732, 99.66896946, 102.63088, 106.5564693, 111.8595076, 116.202345, 120.2027831, 124.410207, 127.7232255, 133.7294309, 136.5622639, 139.8088919, 145.8912633, 148.9342422, 151.4933385, 156.0603218, 158.067154, 162.5681607, 167.1394201, 170.1198513, 174.2094609, 176.9823241, 180.1039229, 182.7409691, 186.6266789, 190.0296273, 194.0574005, 196.2143683, 199.8983893, 202.9530689, 206.3599834, 210.3258442, 213.1789189, 215.0585729, 218.608384, 222.442728, 224.3927411, 228.3649733, 231.5689398, 234.221249, 238.755964, 241.1279941, 242.8026604, 245.0381432, 249.2273827, 251.741933, 255.373651, 257.1924355, 258.943511, 262.645107, 264.5341063, 267.2624818, 270.1318905, 271.8803723, 273.840074, 277.2729129, 279.0226465, 282.3858207, 284.8389007, 287.4314896, 289.9556873, 291.2176193, 292.9007144, 297.3206983, 298.2379459, 300.3435431, 303.3575402, 304.6976579, 305.7498618, 308.4146391, 310.0338754, 312.5637963, 314.6716034, 316.7806703, 318.0459985, 318.820918, 321.3522521, 323.9543434, 325.5671327, 327.3318781, 329.5162233, 331.1364384, 332.3995075, 334.5123231, 336.2027344, 337.7546386, 339.6570911, 340.8526373, 342.336653, 344.0970126, 344.658269, 347.4813688, 348.1163698, 350.089462, 350.8668725, 352.4905559, 353.4750323, 355.4540908, 356.5820025, 357.2928404, 358.9110238, 360.1817023, 361.1718473, 362.3004049, 363.1475207, 364.2828504, 365.6213464, 366.5457091, 367.5282966, 369.0818008, 369.7935289, 370.2841512, 372.1264526, 372.1264526, 372.8285661, 373.6102068, 375.3080689, 375.73176, 376.3685598, 377.2914587, 378.1398469, 379.0571708, 378.9145251, 379.9753989, 381.0363854, 380.8240151, 381.5349404, 382.2409958, 383.020913, 383.7959698, 384.0075831, 384.5071066, 384.9320084, 385.2814585, 385.7810457, 385.7810457, 386.5554963, 386.0706373, 386.6301552, 386.4956055, 387.0551473, 386.842649, 387.3447758, 387.6926693, 387.9741108, 387.6926693, 388.0504214, 387.9823163, 387.7697982, 387.5572848, 387.3447758, 387.8387225, 387.4801574, 387.4136971, 387.2011912, 387.063352, 386.7080964, 386.4956055, 385.6456871, 386.4266905, 364.434292, 376.0186017, 382.1056851, 398.9548778, 409.3778556, 418.823974, 424.9256691, 433.0964601, 416.1139448, 413.9854455, 422.5054579, 420.3747737, 422.9978521, 420.6472586, 416.8882815, 419.5038597, 415.4573281, 414.6740522, 411.5481425, 403.9578981, 394.6719694, 365.947197, 369.4659147, 386.2946048, 389.2665464, 387.3554422, 388.9768622, 382.2524768, 377.9398339, 376.0226995, 375.1089321, 374.114127, 374.048575, 376.8054098, 376.9406487, 377.0168751, 373.6249553, 370.7223735, 370.5790277, 369.3070257, 368.0351857, 366.8322872, 365.847312, 365.627896, 365.4928102, 364.8575221, 364.9344749, 365.847312, 365.6360831, 364.5759124, 363.4438236, 363.5093061, 361.7479768, 360.8330594, 360.2586192, 359.2734739, 358.5649494, 357.0826104, 356.9378241 };


void setup() {
  Serial.begin(115200);
  while (!Serial);
  delay(800);

  for (size_t i = 0; i < sizeof(altitudes) / sizeof(float); i++) {
    float altitude = altitudes[i];
    trajectory.update(altitude);

    Serial.print(altitude);
    Serial.print(",");
    Serial.print(trajectory.climbIndex() * 50);
    Serial.print(",");
    Serial.println(trajectory.isFalling() * 100);
    delay(50);
  }

  Serial.println("finish");
}


void loop() {}