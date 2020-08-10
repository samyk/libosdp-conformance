/*
  oo-pd73 - PD emulator for extended packet mode (PIV) credential processing.

  (C)Copyright 2020 Smithee Solutions LLC
*/


#include <string.h>


#include <open-osdp.h>
#include <osdp_conformance.h>


int
  action_osdp_CRAUTH
    (OSDP_CONTEXT *ctx,
    OSDP_MSG *msg)

{ /* action_osdp_CRAUTH */

  OSDP_MULTI_HDR_IEC *crauth_header;
  char *crauth_payload;
  int current_length;
  int current_security;
  OSDP_MULTI_HDR_IEC *resp_hdr;
  int response_length;
  unsigned char response_payload [2048];
  int rlth;
  int status;


  status = ST_OK;

  // whatever else we think, the PD saw the CRAUTH

  osdp_test_set_status(OOC_SYMBOL_cmd_crauth, OCONFORM_EXERCISED);

  crauth_header = (OSDP_MULTI_HDR_IEC *)(msg->data_payload);
  crauth_payload = (char *)&(crauth_header->algo_payload);

  fprintf(ctx->log,
"  CRAUTH hdr: tlsb %02x tmsb %02x offlsb %02x offmsb %02x lenlsb %02x lenmsb %02x\n",
    crauth_header->total_lsb, crauth_header-> total_msb, crauth_header-> offset_lsb,
    crauth_header-> offset_msb, crauth_header-> data_len_lsb, crauth_header-> data_len_msb);
  fprintf(ctx->log, "CRAUTH Algo/Key %02x Payload %02x%02x%02x...\n",
    crauth_header->algo_payload,
    *(crauth_payload), *(1+crauth_payload), *(2+crauth_payload));

  memset(response_payload, 0, sizeof(response_payload));
  resp_hdr = (OSDP_MULTI_HDR_IEC *)&(response_payload [0]);
rlth=256;
  resp_hdr->total_lsb = (rlth & 0xff);
  resp_hdr->total_msb = (rlth/256);
  // offset is already 0
  resp_hdr->data_len_lsb = (rlth & 0xff);
  resp_hdr->data_len_msb = (rlth/256);
  response_length = rlth + sizeof(resp_hdr);

  current_length = 0;
  current_security = OSDP_SEC_SCS_18;
  status = send_message_ex(ctx, OSDP_CRAUTHR, ctx->pd_address, &current_length,
    response_length, response_payload, current_security, 0, NULL);
fprintf(stderr, "DEBUG: CRAUTHR sent\n");

  return(status);

} /* action_osdp_CRAUTH */


int
  action_osdp_CRAUTHR
    (OSDP_CONTEXT *ctx,
    OSDP_MSG *msg)

{ /* action_osdp_CRAUTHR */

  OSDP_MULTI_HDR_IEC *crauthr_header;
  char *crauthr_payload;
  char details [4*2048]; // ...
  int i;
  int response_length;
  char response_payload [2048]; // assumed to be larger than the whole response...
  int status;


fprintf(ctx->log, "DEBUG: osdp_CRAUTHR stub.\n");
  crauthr_header = (OSDP_MULTI_HDR_IEC *)(msg->data_payload);
  crauthr_payload = (char *)&(crauthr_header->algo_payload);

  fprintf(ctx->log,
"  CRAUTHR received: tlsb %02x tmsb %02x offlsb %02x offmsb %02x lenlsb %02x lenmsb %02x\n",
    crauthr_header->total_lsb, crauthr_header-> total_msb, crauthr_header-> offset_lsb,
    crauthr_header-> offset_msb, crauthr_header-> data_len_lsb, crauthr_header-> data_len_msb);
fprintf(ctx->log, "  crauthr payload %02x%02x%02x...\n",
    *(crauthr_payload), *(crauthr_payload+1), *(crauthr_payload+2));

  response_length = (crauthr_header->total_msb * 256) + crauthr_header->total_lsb;
  details [0] = 0;
  status = ST_OK; // assume for now the header has been validated.
  if (response_length > OSDP_OFFICIAL_MSG_MAX)
    status = ST_OSDP_CRAUTHR_HEADER;

  if (status EQUALS ST_OK)
  {
    memset(response_payload, 0, sizeof(response_payload));
    for (i=0; i<response_length; i++)
    {
      sprintf(response_payload+(2*i), "%02x", (unsigned)*(crauthr_payload+i));
    };
    sprintf(details, "\"crauthr-response\":\"%s\",", response_payload);
    osdp_test_set_status_ex(OOC_SYMBOL_resp_crauthr, OCONFORM_EXERCISED, details);
  }
  else
  {
    osdp_test_set_status_ex(OOC_SYMBOL_resp_crauthr, OCONFORM_FAIL, details);
  };

  return(status);

} /* action_osdp_CRAUTHR */

