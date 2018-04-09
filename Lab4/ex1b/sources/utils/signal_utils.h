#pragma once

int set_signal_handlers();
void SIGSTP_handler(int sign_no);
void SIGINT_handler(int sing_no);